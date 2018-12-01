//File: HistogramWindow.cpp
//Brief: A HistogramWindow histograms the values in a column of a TreeNode<> hierarchy
//       using Dear imgui.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//local includes
#include "HistogramWindow.h"

//imgui includes
#include "imgui_internal.h"

//c++ includes
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace gui
{
  HistogramWindow::HistogramWindow(): fIncludeTopNodes(false), fLogX(false), fLogY(false) {}

  void HistogramWindow::doDrawHistogram(std::vector<std::pair<std::string, float>>&& bins, const std::string& name)
  {
    if(fLogY) for(auto& bin: bins) bin.second = (bin.second>0)?std::log10(bin.second):0; //Protection against log(0) and log(negative number)
    //Nothing to do for log x-axis with strings

    //Render histogram the hard way
    ImVec2 graph_size(600, 400);
    const auto mostEntries = std::max_element(bins.begin(), bins.end(), 
                                              [](const auto& first, const auto& second)
                                              { return first.second < second.second; });
    const auto leastEntries = std::min_element(bins.begin(), bins.end(), 
                                               [](const auto& first, const auto& second)
                                               { return first.second < second.second; });
    const float scale_min = (leastEntries == bins.end())?0:(leastEntries->second<0)?leastEntries->second:0, 
                scale_max = (mostEntries == bins.end())?0:mostEntries->second; //y scale limits
    const int values_count = bins.size(); //Numbers of bins for x axis
    constexpr auto tickLength = 8.f;

    auto& g = *ImGui::GetCurrentContext();
    const auto& style = g.Style;

    const auto maxLabel = std::to_string(scale_max);
    const auto longestYLabel = ImGui::CalcTextSize(maxLabel.c_str()).x;
    const auto xLabelSpace = ImGui::GetTextLineHeight();
    const ImVec2 cursor(ImGui::GetCursorPos().x + ImGui::GetWindowPos().x, ImGui::GetCursorPos().y + ImGui::GetWindowPos().y);
    const ImRect frame_bb(ImVec2(cursor.x + longestYLabel + tickLength, cursor.y - xLabelSpace*2 - tickLength),
                          ImVec2(cursor.x + graph_size.x, cursor.y + graph_size.y));
    const ImRect inner_bb(ImVec2(frame_bb.Min.x + style.FramePadding.x, frame_bb.Min.y + style.FramePadding.y),
                          ImVec2(frame_bb.Max.x - style.FramePadding.x, frame_bb.Max.y - style.FramePadding.y));
    const ImRect total_bb(ImVec2(frame_bb.Min.x + longestYLabel, frame_bb.Min.y), ImVec2(frame_bb.Max.x, frame_bb.Max.y - xLabelSpace));
    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, 0, &frame_bb))
    {
        std::cout << "Returning early for weird ImGui reason that I don't understand.\n";
        return; //TODO: What to do here?
    }
    const bool hovered = ImGui::ItemHoverable(inner_bb, 0);

    //Figure out how many x and y axis labels will fit around this histogram and draw that many
    const auto longestXLabel = ImGui::CalcTextSize(std::prev(bins.end())->first.c_str()).x;
    const size_t nMaxXLabels = (inner_bb.Max.y - inner_bb.Min.y)/longestXLabel;
    const size_t nXTicks = std::min(bins.size(), nMaxXLabels);
    const size_t nMaxYLabels = (inner_bb.Max.x - inner_bb.Min.x)/ImGui::GetTextLineHeight();
    const size_t nYTicks = fLogY?nMaxYLabels/5:std::min(scale_max - scale_min, (float)(nMaxYLabels/5)); //5 chosen empirically

    ImGui::RenderFrame(frame_bb.Min, frame_bb.Max, ImGui::GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);

    if (values_count > 0)
    {
        int res_w = ImMin((int)graph_size.x, values_count);
        int item_count = values_count;

        // Tooltip on hover
        int v_hovered = -1;
        if (hovered)
        {
            const float t = ImClamp((g.IO.MousePos.x - inner_bb.Min.x) / (inner_bb.Max.x - inner_bb.Min.x), 0.0f, 0.9999f);
            const int v_idx = (int)(t * (item_count));
            IM_ASSERT(v_idx >= 0 && v_idx < values_count);
   
            auto key = bins.begin();
            std::advance(key, (v_idx) % values_count); 
            ImGui::SetTooltip((key->first+": "+std::to_string(key->second)).c_str()); //TODO: Change tooltip to bin lower limit
            v_hovered = v_idx;
        }
    
        const float t_step = 1.0f / (float)res_w;
        const float inv_scale = (scale_min == scale_max) ? 0.0f : (1.0f / (scale_max - scale_min));
    
        float v0 = bins.begin()->second;
        float t0 = 0.0f;
        ImVec2 tp0 = ImVec2( t0, 1.0f - ImSaturate((v0 - scale_min) * inv_scale) );                       // Point in the normalized space of our target rectangle
        float histogram_zero_line_t = (scale_min * scale_max < 0.0f) ? (-scale_min * inv_scale) : (scale_min < 0.0f ? 0.0f : 1.0f);   // Where does the zero line stands
    
        const ImU32 col_base = ImGui::GetColorU32(ImGuiCol_PlotHistogram);
        const ImU32 col_hovered = ImGui::GetColorU32(ImGuiCol_PlotHistogramHovered);
        const auto axis_color = ImGui::ColorConvertFloat4ToU32(ImVec4(0.f, 0.f, 0.f, 1.f));
    
        const size_t xBinsPerTick = bins.size()/nXTicks;
        auto currentBin = bins.begin();
        for (int n = 0; n < res_w; n++)
        {
            const float t1 = t0 + t_step;
            const int v1_idx = (int)(t0 * item_count + 0.5f);
            IM_ASSERT(v1_idx >= 0 && v1_idx < values_count);
            const auto binForNextValue = (std::next(currentBin) == bins.end())?bins.begin():std::next(currentBin); 
            const float v1 = binForNextValue->second;
            const ImVec2 tp1 = ImVec2( t1, 1.0f - ImSaturate((v1 - scale_min) * inv_scale) );
    
            // NB: Draw calls are merged together by the DrawList system. Still, we should render our batch are lower level to save a bit of CPU.
            ImVec2 pos0 = ImLerp(inner_bb.Min, inner_bb.Max, tp0);
            ImVec2 pos1 = ImLerp(inner_bb.Min, inner_bb.Max, ImVec2(tp1.x, histogram_zero_line_t));
            if (pos1.x >= pos0.x + 2.0f)
                pos1.x -= 1.0f;
            ImGui::GetWindowDrawList()->AddRectFilled(pos0, pos1, v_hovered == v1_idx ? col_hovered : col_base);

            if(v1_idx % xBinsPerTick == 0)
            {
              //Draw x axis label
              const auto labelSize = ImGui::CalcTextSize(currentBin->first.c_str());
              ImGui::RenderText(ImVec2((pos0.x + pos1.x)/2.f - labelSize.x/2.f, 
                                       inner_bb.Max.y + ImGui::GetTextLineHeight()/2.f), currentBin->first.c_str());

              //Draw an x tick mark
              ImGui::GetWindowDrawList()->AddLine(ImVec2((pos0.x + pos1.x)/2.f, inner_bb.Max.y - tickLength),
                                                  ImVec2(ImVec2((pos0.x + pos1.x)/2.f, inner_bb.Max.y + tickLength)),
                                                  axis_color);
            }

            t0 = t1;
            tp0 = tp1;
            ++currentBin; //We should be protected from going past the end of bins by the condition on n
        }

        //Draw x axis
        ImGui::GetWindowDrawList()->AddLine(ImVec2(inner_bb.Min.x, inner_bb.Max.y), inner_bb.Max, axis_color); //Axis
        const auto nameLength = ImGui::CalcTextSize(name.c_str()).x; //Center text
        ImGui::RenderText(ImVec2((inner_bb.Min.x + inner_bb.Max.x)/2. - nameLength/2.f,
                                 inner_bb.Max.y + ImGui::GetTextLineHeight()*3.f/2.f + tickLength), name.c_str());//Label

        //Draw y axis
        ImGui::GetWindowDrawList()->AddLine(ImVec2(inner_bb.Min.x, inner_bb.Max.y), inner_bb.Min, axis_color);

        //Draw y tick marks
        const float step = (scale_max - scale_min)/nYTicks;
        for(size_t tick = 0; tick < nYTicks; ++tick)
        {
          const ImVec2 pos = ImVec2( inner_bb.Min.x, 1.0f - ImSaturate((tick*step - scale_min) * inv_scale) );
          const auto yPos = ImLerp(inner_bb.Min, inner_bb.Max, pos).y;

          ImGui::GetWindowDrawList()->AddLine(ImVec2(inner_bb.Min.x - tickLength, yPos), ImVec2(inner_bb.Min.x + tickLength, yPos), axis_color);

          const auto label = fLogY?std::to_string(tick*step+scale_min):std::to_string((size_t)(tick*step+scale_min)); 
          const auto labelSize = ImGui::CalcTextSize(label.c_str());
          ImGui::RenderText(ImVec2(inner_bb.Min.x - labelSize.x - 1.5f*tickLength, yPos - ImGui::GetTextLineHeight()/2.f), label.c_str());
        }
    }
  }
}
