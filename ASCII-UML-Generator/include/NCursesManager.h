#pragma once

#include <ncurses.h>

#include "Buffer.h"
#include "Pos.h"

struct MouseEvent
{
    MouseEvent(const Pos& pos_, int eventType_)
        : pos(pos_), eventType(static_cast<EventType>(eventType_))
    {
    }
    const Pos pos;
    const enum class EventType : uint32_t {
        Pressed = BUTTON1_PRESSED,
        Released = BUTTON1_RELEASED
    } eventType;
};

class NCursesManager
{
public:
    NCursesManager()
    {
        initscr();
        clear();
        noecho();
        cbreak();

        // Enables keypad mode. This makes mouse events getting
        // reported as KEY_MOUSE, instead as of random letters.
        keypad(stdscr, TRUE);

        // Don't mask any mouse events
        //    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
        mousemask(ALL_MOUSE_EVENTS, NULL);

        // From https://gist.github.com/sylt/93d3f7b77e7f3a881603
        // printf("\033[?1003h\n") enable reporting of the mouse position
        // (see console_codes(4) for more information)
        //    printf("\033[?1003h\n");
    }

    ~NCursesManager()
    {
        clrtoeol();
        refresh();
        endwin();

        // Disable mouse movement events, as l = low
        //    printf("\033[?1003l\n");
    }

    void printOnScreen(int x_, int y_, const std::string& message_) const
    {
        mvprintw(y_, x_, message_.c_str());
    }

    void refresh() { ::refresh(); }

    MouseEvent getMouseEvent()
    {
        while (true)
        {
            const int ch = wgetch(stdscr);
            if (ch == KEY_MOUSE)
            {
                MEVENT mouse_event{};
                if (getmouse(&mouse_event) == OK)
                {
                    if (mouse_event.bstate & BUTTON1_RELEASED or
                        mouse_event.bstate & BUTTON1_PRESSED)
                    {
                        return MouseEvent({mouse_event.x, mouse_event.y},
                                          mouse_event.bstate);
                    }
                }
            }
        }
    }

    void render(const Buffer& buf)
    {
        for (size_t y = 0; y != buf.size(); ++y)
        {
            for (size_t x = 0; x != buf[0].size(); ++x)
            {
                const auto buf_elem = buf[y][x];
                // DEBUG
                //                        char c{};
                //                        switch (buf_elem.type())
                //                        {
                //                        case Buffer::ElemType::Box:
                //                             c = 'B';
                //                            break;
                //                        case Buffer::ElemType::Arrow:
                //                            c = 'A';
                //                            break;
                //                        default:
                //                            c = buf_elem;
                //                            break;
                //                        }

                const int c = buf_elem ? buf_elem : ' ';

                mvaddch(y, x, c);
            }
        }
    }
};
