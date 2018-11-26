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
#include <array>

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

    template <int NBINS>
    std::array<COMPARABLE, NBINS> BinData()
    {
      static_assert(NBINS > 0, "Requested negative number of bins in HistogramModel<>::BinData()");
      unitsPerBin = max/(NBINS-1);
      std::array<COMPARABLE, NBINS> bins = {0};
      for(const auto& value: values) ++bins.at((value < 0)?0:(value/unitsPerBin)); 
      return bins;
    }

    COMPARABLE unitsPerBin;
    COMPARABLE min;
    COMPARABLE max;

    private:
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

    std::map<std::string, size_t> BinData() const
    {
      return fBins;
    }

    private:
      std::map<std::string, size_t> fBins;
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
                             model(this->fLogX?log(value):value); //TODO: Handle negative values gracefully when in log mode
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
        if(open) doDrawHistogram(std::move(model), name);

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
      void doDrawHistogram(HistogramModel<float>&& model, const std::string& name);
      void doDrawHistogram(HistogramModel<std::string>&& model, const std::string& name);

      //Data for histogram options
      bool fIncludeTopNodes; //Should top-level nodes be included in histograms?
      bool fLogX; //Should the x axis have a log scale?
      bool fLogY; //Should the y axis have a log scale?
  };
}

#endif //GUI_HISTOGRAMWINDOW_H
