#include <wx/wx.h>

#include "lost_levels/engine.h"
#include "lost_levels/graphics_sdl2.h"
#include "lost_levels/timer_sdl2.h"
#include "lost_levels/diag.h"
#include "lain/settings.h"

#include "editor.h"

#pragma clang diagnostic ignored "-Wswitch"

using namespace std;
using namespace lain;
using namespace lost_levels;
using namespace lost_levels::editor;

static const wxString WINDOW_TITLE = _T("Lost Levels Tile Editor");

MainFrame::MainFrame(wxWindow* parent, int id, const wxString& title) :
   wxFrame(parent, id, title) {

   mainVerticalSplit = new wxSplitterWindow(this);
   canvasPanel = new wxPanel(mainVerticalSplit);

}

class EditorApp : public wxApp {
public:
   bool OnInit() override {
      MainFrame* frame = new MainFrame(nullptr, wxID_ANY, WINDOW_TITLE);
      frame->Show(true);
      return true;
   }
};
