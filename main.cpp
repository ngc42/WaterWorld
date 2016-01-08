/* This file is part of WaterWorld.
 * License GPL Version 3, see file "LICENSE" for details
 * Author Eike Lange
 */

#include <SFML/Graphics.hpp>
#include <iostream>
#include <functional> // std::bind

#include "isle.h"
#include "ship.h"
#include "screens.h"
#include "manager.h"


int main()
{
    sf::RenderWindow window(sf::VideoMode(800, 400), "Galactic", sf::Style::Titlebar);
    Manager manager(600, 400, 5, 1);

    InfoScreen infoscreen(
                600, /* 0, */ 200, 400,
                std::bind(ship_message_function,
                          manager,
                          std::placeholders::_1,
                          std::placeholders::_2));
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if(event.type == sf::Event::MouseButtonPressed)
            {
                // -- // std::cout << "Mouse button pressed: ";
                // -- // std::cout << event.mouseButton.x << " " << event.mouseButton.y << std::endl;

                bool foundClickTarget = infoscreen.pointInfoscreen(event.mouseButton.x);

                if(foundClickTarget) /* clicked infoscreen */
                    infoscreen.processInfoscreenClick(event.mouseButton.x, event.mouseButton.y);
                else /* clicked isle, ship or ocean */
                {
                    IsleInfo iInfo;
                    foundClickTarget = manager.isleInfoByPos(iInfo, event.mouseButton.x, event.mouseButton.y);

                    if(foundClickTarget) /* clicked isle */
                    {
                        if(iInfo.owner == 1) /* Human player */
                        {
                            TShipInfos infos;
                            manager.shipInfosByIsleInfo(infos, iInfo);
                            infoscreen.showHumanIsle(iInfo, infos);
                        }
                        else /* other player or empty isle */
                            infoscreen.showIsle(iInfo);
                    }
                    else /* ship or ocean */
                    {
                        ShipInfo sInfo;
                        foundClickTarget = manager.shipInfoByPos(sInfo, event.mouseButton.x, event.mouseButton.y);
                        if(foundClickTarget) /* clicked ship */
                            infoscreen.showShip(sInfo);
                        else /* clicked ocean */
                            infoscreen.showUniverse( event.mouseButton.x, event.mouseButton.y );

                    }
                }
            }
            else if(event.type == sf::Event::KeyPressed)
            {
                std::cout << "Key pressed" << std::endl;
                window.close();
            }

            else if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }
        window.clear();
        // draw isles
        for(Isle *isle : manager.m_isles)
        {
            window.draw(isle->shape());
        }
        // draw ships on ocean
        for(Ship *ship : manager.m_ships)
        {
            if(ship->postionType() == TShipPosType::S_OCEAN)
                window.draw(ship->shape());
        }

        infoscreen.paint(window);
        window.display();
    }
    return 0;
}
