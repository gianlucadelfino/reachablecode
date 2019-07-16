#include <array>
#include <cassert>
#include <fstream>
#include <iostream>
#include <memory>
#include <queue>
#include <set>
#include <stdio.h>
#include <utility>
#include <vector>

#include "Buffer.h"
#include "ClassDiagram.h"
#include "ClassNode.h"
#include "Drawing.h"
#include "NCursesManager.h"
#include "TimeLogger.h"

int main()
{
    std::ofstream stats("times.txt");

    //     _________           ______________
    //    | MyClass |<>-------| MyOtherClass |
    //     ---------           --------------
    Buffer buffer;

    ClassDiagram diagram(buffer);

    NCursesManager ncursesManager;

    ncursesManager.printOnScreen(
        0, 0, "Use arrow keys to select the class to move, then press enter");

    Pos delta_start;

    while (true)
    {
        {
            TimeLogger t("render", stats);
            ncursesManager.render(buffer);
        }

        MouseEvent event = ncursesManager.getMouseEvent();

        if (event.eventType == MouseEvent::EventType::Released)
        {
            Pos delta_end(event.pos.x, event.pos.y);
            mvprintw(
                0, 0, "Released button %d %d      ", delta_end.x, delta_end.y);
            {
                TimeLogger t("move", stats);
                diagram.moveClass(delta_start, delta_end);
            }
            {
                TimeLogger t("draw", stats);
                diagram.draw(buffer);
            }
        }
        else if (event.eventType == MouseEvent::EventType::Pressed)
        {
            delta_start = Pos(event.pos.x, event.pos.y);
            mvprintw(0,
                     0,
                     "Mouse PRESSED Event %d %d   ",
                     delta_start.x,
                     delta_start.y);
        }
        else
        {
            // Not working
            //            if (false)
            //            {
            //                Pos delta_end(event.pos.x, event.pos.y);

            //                if (delta_end != delta_start)
            //                {
            //                    mvprintw(0,
            //                             0,
            //                             "Released button %d %d",
            //                             delta_end.x,
            //                             delta_end.y);
            //                    {
            //                        TimeLogger t("move", stats);
            //                        diagram.moveClass(delta_start, delta_end);
            //                    }
            //                    {
            //                        TimeLogger t("draw", stats);
            //                        diagram.draw(buffer);
            //                    }
            //                    delta_start = event.pos;
            //                }
            //            }
            //            else
            //            {
            //                mvprintw(2,
            //                         0,
            //                         "Mouse pos %d %d id %d",
            //                         event.pos.x,
            //                         event.pos.y,
            //                         static_cast<uint32_t>(event.eventType));
            //            }
            //                            }
            //                        }
        }

        {
            TimeLogger t("refresh", stats);
            ncursesManager.refresh();
        }
    }

    return 0;
}
