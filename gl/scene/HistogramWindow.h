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
#include <set>
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

    private:
      std::vector<float> values;
      COMPARABLE min;
      COMPARABLE max;
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
      fBins.insert(name);
    }

    std::multiset<std::string> BinData() const
    {
      return fBins;
    }

    private:
      std::multiset<std::string> fBins;
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
          for(const auto& top: nodes)
          {
            top.walkWhileTrue([&model, &col](const auto& node)
                              {
                                if(!node.fVisible) return false; //Only plot visible nodes
                                //TODO: Plot all nodes in another color
                                
                                model(std::stof(node.row[col]));              
                                return true;
                              });
          }

          return DrawHistogram(std::move(model), name);
        }
        catch(const std::invalid_argument& e)
        {
          std::cerr << "When trying to histogram " << name << ", got a value that is not convertible to float:\n"
                    << e.what() << "\nTrying to plot strings instead.\n";

          HistogramModel<std::string> model;
          for(const auto& top: nodes)
          {
            top.walkWhileTrue([&model, &col](const auto& node)
                              {
                                if(!node.fVisible) return false; //Only plot visible nodes
                                //TODO: Plot all nodes in another color

                                model(node.row[col]);
                                return true;
                              });
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
        if(!open) return false;

        doDrawHistogram(std::move(model), name);
        ImGui::End();
        return true;
      }

      //Actually draw the histogram here.
      void doDrawHistogram(HistogramModel<float>&& model, const std::string& name);
      void doDrawHistogram(HistogramModel<std::string>&& model, const std::string& name);

      //Data for histogram options
  };
}

#endif //GUI_HISTOGRAMWINDOW_H
