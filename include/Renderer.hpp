#include <User_Setup.h>
#include <TFT_eSPI.h>

namespace Renderer
{
    extern TFT_eSPI tft;

    void init();

    void updateDisplay();

    void initPanels();
}