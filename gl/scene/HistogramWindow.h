//File: HistogramWindow.h
//Brief: A HistogramWindow displays a histogram of values in a hierarchy of TreeNodes.  
//       Render() it to draw the histogram itself via Dear imgui.  I started off by 
//       using ImGui::PlotHistogram() to implement histogram drawing, then I copied 
//       ImGui::PlotEx() to do my own custom histogram rendering.  
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef GUI_HISTOGRAMWINDOW_H
#define GUI_HISTOGRAMWINDOW_H

//imgui includes
#include "imgui.h"

//c++ includes
#include <list>
#include <map>
#include <vector>
#include <cstdlib>
#include <string>
#include <limits>
#include <stdexcept>
#include <iostream>
#include <cmath>

namespace gui
{
  template <class COMPARABLE>
  struct HistogramModel
  {
    HistogramModel(): min(std::numeric_limits<COMPARABLE>::max()), max(std::numeric_limits<COMPARABLE>::min()) {}
    ~HistogramModel() = default; 

    void operator()(const COMPARABLE value)
    {
      values.push_back(value);
      if(value < min) min = value;
      if(value > max) max = value;
    }

    std::vector<std::pair<std::string, float>> BinData(const size_t nBins)
    {
      assert(nBins > 1);
      assert(max > min); //This will also fail of values is empty
      const auto unitsPerBin = (max-min)/(nBins-1);

      //Set up bins based on requested number of bins and min, max
      std::map<float, float> bins; 
      for(size_t bin = 0; bin < nBins; ++bin)
      {
        bins[bin * unitsPerBin + min] = 0;
      }

      //Bin values
      for(const auto& value: values) 
      {
        assert(bins.upper_bound(value) != bins.begin());
        ++(std::prev(bins.upper_bound(value))->second); //Last bin also gets overflows
      }

      //Stringify bin lower bounds
      std::vector<std::pair<std::string, float>> result;
      for(const auto& bin: bins)
      {
        result.emplace_back(std::to_string(bin.first), bin.second);
      }

      return result;
    }

    private:
      COMPARABLE min;
      COMPARABLE max;
      std::vector<float> values;
  };

  //Each unique string gets its own bin.
  //No need to keep min and max limits or number of bins for strings.  
  template <>
  struct HistogramModel<std::string>
  {
    HistogramModel() = default;
    ~HistogramModel() = default;
     
    void operator()(const std::string& name)
    {
      ++fBins[name];
    }

    std::vector<std::pair<std::string, float>> BinData(const size_t /*nBins*/) const
    {
      std::vector<std::pair<std::string, float>> binView;
      for(const auto& bin: fBins) binView.emplace_back(bin.first, bin.second);
      return binView;
    }

    private:
      std::map<std::string, float> fBins;
  };

  class HistogramWindow
  {
    public:
      HistogramWindow();
      virtual ~HistogramWindow() = default;

      //Return value indicates whether window is open
      template <class NODE>
      bool Render(const std::list<NODE>& nodes, const size_t col, const std::string& name)
      {
        try
        {
          //Get data to histogram
          HistogramModel<float> model; 
          //TODO: Those if() statements seem to significantly hurt performance of the application when I'm drawing a histogram.
          //      Since I can't have interchangeable lambdas here, I should probably write some 
          //      functor that does the model-filling and write versions with and without taking log.
          auto fillFloat = [&model, &col, this](const auto& node)
                           {
                             if(!node.fVisible) return false; //Only plot visible nodes
                             //TODO: Plot all nodes in another color

                             const auto value = std::stof(node.row[col]);
                             if(fLogX) 
                             {
                               if(value > 0) model(std::log10(value));
                               else 
                               {
                                 std::cerr << "Suppressing value " << value << " that is <= 0 because drawing log(x axis).\n";
                               }
                             }
                             else model(value); //TODO: Handle negative values gracefully when in log mode
                             return true;
                           };

          for(const auto& top: nodes)
          {
            if(fIncludeTopNodes)
            {
              top.walkWhileTrue(fillFloat);
            }
            else
            {
              for(const auto& child: top.children)
              {
                child.walkWhileTrue(fillFloat);
              }
            }
          }

          return DrawHistogram(std::move(model), name);
        }
        catch(const std::invalid_argument& e)
        {
          /*std::cerr << "When trying to histogram " << name << ", got a value that is not convertible to float:\n"
                    << e.what() << "\nTrying to plot strings instead.\n";*/

          HistogramModel<std::string> model;
          const auto fillString = [&model, &col](const auto& node)
                                  {
                                    if(!node.fVisible) return false; //Only plot visible nodes
                                    //TODO: Plot all nodes in another color

                                    model(node.row[col]);
                                    return true;
                                  };

          for(const auto& top: nodes)
          {
            if(fIncludeTopNodes)
            {
              top.walkWhileTrue(fillString);
            }
            else
            {
              for(const auto& child: top.children)
              {
                child.walkWhileTrue(fillString);
              }
            }
          }

          return DrawHistogram(std::move(model), name);
        }
      }

    private:
      //Create a window for histogram and controls.  Return whether window is still open
      template <class COMPARABLE>
      bool DrawHistogram(HistogramModel<COMPARABLE>&& model, const std::string& name)
      {
        bool open = true;
        ImGui::Begin(name.c_str(), &open);
 
        constexpr size_t nBins = 100;
        if(open) doDrawHistogram(model.BinData(nBins), name);

        //Drawing histogramming control GUI
        ImGui::NewLine(); //TODO: Understand why axis labels aren't recognized as a line
        ImGui::Checkbox("Include Top-level Nodes in Plots", &fIncludeTopNodes);
        ImGui::SameLine();
        ImGui::Checkbox("Log x axis", &fLogX);
        ImGui::SameLine();
        ImGui::Checkbox("Log y axis", &fLogY);

        ImGui::End();
        return open;
      }

      //Actually draw the histogram here.
      void doDrawHistogram(std::vector<std::pair<std::string, float>>&& bins, const std::string& name);

      //Data for histogram options
      bool fIncludeTopNodes; //Should top-level nodes be included in histograms?
      bool fLogX; //Should the x axis have a log scale?
      bool fLogY; //Should the y axis have a log scale?
  };
}

#endif //GUI_HISTOGRAMWINDOW_H
