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

  void HistogramWindow::doDrawHistogram(HistogramModel<float>&& model, const std::string& name)
  {
    //Prepare data in bins that Dear ImGui can deal with
    constexpr auto nBins = 100;
    auto bins = model.BinData<nBins>();
    const auto unitsPerBin = model.unitsPerBin;
    const bool useScientific = (model.min < 1) || (model.max > 1e4);

    if(fLogY) for(auto& bin: bins) bin = std::log(bin);

    //Render histogram the hard way
    constexpr auto nTicks = 10; 
    ImVec2 graph_size(600, 400);
    const auto mostEntries = std::max_element(bins.begin(), bins.end());
    const float scale_min = 0, scale_max = (mostEntries == bins.end())?0:*mostEntries; //y scale limits
    const int values_count = bins.size(); //Numbers of bins for x axis

    auto& g = *ImGui::GetCurrentContext();
    const auto& style = g.Style;

    const ImVec2 cursor(ImGui::GetCursorPos().x + ImGui::GetWindowPos().x, ImGui::GetCursorPos().y + ImGui::GetWindowPos().y);
    const ImRect frame_bb(cursor, ImVec2(cursor.x + graph_size.x, cursor.y + graph_size.y));
    const ImRect inner_bb(ImVec2(frame_bb.Min.x + style.FramePadding.x, frame_bb.Min.y + style.FramePadding.y), 
                          ImVec2(frame_bb.Max.x - style.FramePadding.x, frame_bb.Max.y - style.FramePadding.y));
    const ImRect total_bb(frame_bb.Min, ImVec2(frame_bb.Max.x, frame_bb.Max.y + ImGui::GetTextLineHeight()));
    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, 0, &frame_bb))
    {
        std::cout << "Returning early for weird ImGui reason that I don't understand.\n";
        return; //TODO: What to do here?
    }
    const bool hovered = ImGui::ItemHoverable(inner_bb, 0);

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
            const int v_idx = (int)(t * item_count);
            IM_ASSERT(v_idx >= 0 && v_idx < values_count);
    
            const float v0 = bins[(v_idx) % values_count];
            ImGui::SetTooltip("%d: %8.4g", (int)(v_idx*unitsPerBin), v0); //TODO: Change tooltip to bin lower limit
            v_hovered = v_idx;
        }
    
        const float t_step = 1.0f / (float)res_w;
        const float inv_scale = (scale_min == scale_max) ? 0.0f : (1.0f / (scale_max - scale_min));
    
        float v0 = bins[0];
        float t0 = 0.0f;
        ImVec2 tp0 = ImVec2( t0, 1.0f - ImSaturate((v0 - scale_min) * inv_scale) );                       // Point in the normalized space of our target rectangle
        float histogram_zero_line_t = (scale_min * scale_max < 0.0f) ? (-scale_min * inv_scale) : (scale_min < 0.0f ? 0.0f : 1.0f);   // Where does the zero line stands
    
        const ImU32 col_base = ImGui::GetColorU32(ImGuiCol_PlotHistogram);
        const ImU32 col_hovered = ImGui::GetColorU32(ImGuiCol_PlotHistogramHovered);
    
        for (int n = 0; n < res_w; n++)
        {
            const float t1 = t0 + t_step;
            const int v1_idx = (int)(t0 * item_count + 0.5f);
            IM_ASSERT(v1_idx >= 0 && v1_idx < values_count);
            const float v1 = bins[(v1_idx + 1)% values_count];
            const ImVec2 tp1 = ImVec2( t1, 1.0f - ImSaturate((v1 - scale_min) * inv_scale) );
    
            // NB: Draw calls are merged together by the DrawList system. Still, we should render our batch are lower level to save a bit of CPU.
            ImVec2 pos0 = ImLerp(inner_bb.Min, inner_bb.Max, tp0);
            ImVec2 pos1 = ImLerp(inner_bb.Min, inner_bb.Max, ImVec2(tp1.x, histogram_zero_line_t));
            if (pos1.x >= pos0.x + 2.0f)
                pos1.x -= 1.0f;
            ImGui::GetWindowDrawList()->AddRectFilled(pos0, pos1, v_hovered == v1_idx ? col_hovered : col_base);

            if(n % (nBins/nTicks) == 0)
            {
              //Draw x axis label
              std::stringstream ss;
              if(useScientific) ss << std::setprecision(2) << (v1_idx*unitsPerBin);
              else ss << std::setprecision(0) << std::fixed << (v1_idx*unitsPerBin);
              const auto label = ss.str();
              const auto labelSize = ImGui::CalcTextSize(label.c_str());
              ImGui::RenderText(ImVec2((pos0.x + pos1.x)/2.f - labelSize.x/2.f,
                                       frame_bb.Max.y + ImGui::GetTextLineHeight()), label.c_str());
            }

            t0 = t1;
            tp0 = tp1;
        }
    }

    //TODO: Allow user to make graphical cuts on histogram.  Then, apply those graphical cuts to 
    //      the current event's model.
    //ImGui::PlotHistogram(name.c_str(), bins.data(), bins.size(), 0, "Overlay Test", 0.0f, FLT_MAX, ImVec2(600, 400));
  }

  void HistogramWindow::doDrawHistogram(HistogramModel<std::string>&& model, const std::string& name)
  {
    auto bins = model.BinData();

    if(fLogY) for(auto& bin: bins) bin.second = std::log(bin.second);
    //Nothing to do for log x-axis with strings

    //Render histogram the hard way
    ImVec2 graph_size(600, 400);
    const auto mostEntries = std::max_element(bins.begin(), bins.end(), 
                                              [](const auto& first, const auto& second)
                                              { return first.second < second.second; });
    const float scale_min = 0, scale_max = (mostEntries == bins.end())?0:mostEntries->second; //y scale limits
    const int values_count = bins.size(); //Numbers of bins for x axis

    auto& g = *ImGui::GetCurrentContext();
    const auto& style = g.Style;

    const ImVec2 cursor(ImGui::GetCursorPos().x + ImGui::GetWindowPos().x, ImGui::GetCursorPos().y + ImGui::GetWindowPos().y);
    const ImRect frame_bb(cursor, ImVec2(cursor.x + graph_size.x, cursor.y + graph_size.y));
    const ImRect inner_bb(ImVec2(frame_bb.Min.x + style.FramePadding.x, frame_bb.Min.y + style.FramePadding.y), 
                          ImVec2(frame_bb.Max.x - style.FramePadding.x, frame_bb.Max.y - style.FramePadding.y));
    const ImRect total_bb(frame_bb.Min, ImVec2(frame_bb.Max.x, frame_bb.Max.y + ImGui::GetTextLineHeight()));
    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, 0, &frame_bb))
    {
        std::cout << "Returning early for weird ImGui reason that I don't understand.\n";
        return; //TODO: What to do here?
    }
    const bool hovered = ImGui::ItemHoverable(inner_bb, 0);

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

            //Draw x axis label
            const auto labelSize = ImGui::CalcTextSize(currentBin->first.c_str());
            ImGui::RenderText(ImVec2((pos0.x + pos1.x)/2.f - labelSize.x/2.f, 
                                     frame_bb.Max.y + ImGui::GetTextLineHeight()), currentBin->first.c_str());

            t0 = t1;
            tp0 = tp1;
            ++currentBin; //We should be protected from going past the end of bins by the condition on n
        }
    }
  }
}
