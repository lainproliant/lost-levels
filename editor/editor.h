#pragma once

#include <wx/wx.h>
#include <wx/splitter.h>
#include <wx/treectrl.h>

namespace lost_levels {
   namespace editor {
      class Toolbox : public wxWindow {

      };

      class Canvas : public wxWindow {

      };

      class MainFrame : public wxFrame {
      public:
         MainFrame(wxWindow* parent, int id, const wxString& title);

      private:
         wxSplitterWindow* mainVerticalSplit;
         wxSplitterWindow* leftHorizontalSplit;
         wxTreeCtrl* contentTree;

         wxPanel* toolboxPanel;
         wxComboBox* toolboxSelector;
         Toolbox* toolbox;

         wxPanel* canvasPanel;
         Canvas* canvas;
      };
   }
}
