#include <array>
#include <cassert>
#include <iostream>
#include <memory>
#include <queue>
#include <set>
#include <stdio.h>
#include <utility>
#include <vector>
#include <fstream>

#include <ncurses.h>

#include "Buffer.h"
#include "ClassDiagram.h"
#include "ClassNode.h"
#include "Drawing.h"
#include "TimeLogger.h"

int main()
{
    std::ofstream stats("times.txt");

    //     _________           ______________
    //    | MyClass |<>-------| MyOtherClass |
    //     ---------           --------------
    Buffer buffer;

    ClassDiagram diagram(buffer);

    const int WIDTH = buffer[0].size();
    const int HEIGHT = buffer.size();

    // Construtor
    //    {
    initscr();
    clear();
    noecho();
    cbreak();
    WINDOW* window = newwin(HEIGHT, WIDTH, 0, 0);
    keypad(window, TRUE);
    mvprintw(
        0, 0, "Use arrow keys to select the class to move, then press enter");
    //    }

    // Run()
    //{
    refresh();
    mousemask(ALL_MOUSE_EVENTS, NULL);
    Pos delta_start;
    int runs = 4;
    while (runs)
    {
        --runs;
        {
            TimeLogger t("render", stats);
            render(buffer);
        }
        refresh();

        const int ch = wgetch(window);
        if (ch == KEY_MOUSE)
        {
            MEVENT mouse_event{};
            if (getmouse(&mouse_event) == OK)
            {
                if (mouse_event.bstate & BUTTON1_RELEASED)
                {
                    Pos delta_end(mouse_event.x, mouse_event.y);
                    mvprintw(0,
                             0,
                             "Released button %d %d",
                             delta_end.x,
                             delta_end.y);
                    {
                        TimeLogger t("move", stats);
                        diagram.moveClass(delta_start, delta_end);
                    }
                    {
                        TimeLogger t("draw", stats);
                        diagram.draw(buffer);
                    }
                }
                else if (mouse_event.bstate & BUTTON1_PRESSED)
                {
                    delta_start = Pos(mouse_event.x, mouse_event.y);
                    mvprintw(0,
                             0,
                             "Mouse PRESSED Event %d %d",
                             delta_start.x,
                             delta_start.y);
                }
                else if (mouse_event.bstate)
                {
                    Pos delta_end(mouse_event.x, mouse_event.y);
                    mvprintw(1, 0, "bstate %d %d", delta_end.x, delta_end.y);
                    //                    diagram.moveClass(delta_start,
                    //                    delta_end); diagram.draw(buffer);
                }
            }
        }
        else
        {

            //        else
            {
                const int y = getcury(window);
                const int x = getcurx(window);
                mvprintw(2, 0, "Mouse pos %d %d", x, y);

                //  if (pressed)
            }

            mvprintw(3,
                     0,
                     "Charcter pressed is = %3d Hopefully it can be "
                     "printed as '%c'",
                     ch,
                     ch);
        }

        {
            TimeLogger t("refresh", stats);
            refresh();
        }
    }

    // Destructor
    clrtoeol();
    refresh();
    endwin();

    return 0;
}
