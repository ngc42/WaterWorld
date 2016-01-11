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
    Manager *manager = new Manager(600, 400, 5, 1);

    InfoScreen infoscreen(
                600, /* 0, */ 200, 400,
                std::bind(ship_message_function,
                          manager,
                          std::placeholders::_1,
                          std::placeholders::_2));

    // mouse and selecting a ship's target
    sf::Texture mouseNormalTexture;
    mouseNormalTexture.loadFromFile("MouseNormal.png");
    sf::Texture mouseSelectTexture;
    mouseSelectTexture.loadFromFile("MouseSelect.png");
    sf::Sprite mouseSprite(mouseNormalTexture);

    bool shipWantsTarget = false;
    window.setMouseCursorVisible(false);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if(event.type == sf::Event::MouseButtonPressed and
               event.mouseButton.button == sf::Mouse::Left)
            {
                bool foundClickTarget = infoscreen.pointInfoscreen(event.mouseButton.x);

                if(foundClickTarget ) /* clicked infoscreen */
                {
                    if(shipWantsTarget) continue;

                    infoscreen.processInfoscreenClick(event.mouseButton.x, event.mouseButton.y);
                    // as a result of clicking on the infosccreen, the manager may know
                    // about a ship, which wants a new target.

                    if(manager->m_shipWithIdWantsANewTarget > 0)
                    {
                        shipWantsTarget = true;
                        mouseSprite.setTexture(mouseSelectTexture);
                    }
                }
                else /* clicked isle, ship or ocean */
                {
                    IsleInfo iInfo;
                    foundClickTarget = manager->isleInfoByPos(iInfo, event.mouseButton.x, event.mouseButton.y);

                    if(foundClickTarget) /* clicked isle */
                    {
                        if(shipWantsTarget)
                        {
                            std::cout << "ship with id " << manager->m_shipWithIdWantsANewTarget <<
                                         "wants target isle with id" <<
                                         iInfo.id << std::endl;
                            // find out isle pos and set this as target for ship
                            sf::Vector2f p;
                            if (manager->islePosById(p, iInfo.id))
                                manager->setTargetForShip(manager->m_shipWithIdWantsANewTarget,
                                                          TTargetType::T_ISLE, iInfo.id, p);
                            // clean up
                            shipWantsTarget = false;
                            manager->m_shipWithIdWantsANewTarget = 0;
                            mouseSprite.setTexture(mouseNormalTexture);
                        }
                        else
                        {
                            if(iInfo.owner == 1) /* Human player */
                            {
                                TShipInfos infos;
                                manager->shipInfosByIsleInfo(infos, iInfo);
                                infoscreen.showHumanIsle(iInfo, infos);
                            }
                            else /* other player or empty isle */
                                infoscreen.showIsle(iInfo);
                        }
                    }
                    else /* ship or ocean */
                    {
                        ShipInfo sInfo;
                        foundClickTarget = manager->shipInfoByPos(sInfo, event.mouseButton.x, event.mouseButton.y);
                        if(foundClickTarget) /* clicked ship */
                        {
                            if(shipWantsTarget)
                            {
                                std::cout << "ship with id " << manager->m_shipWithIdWantsANewTarget <<
                                             "wants target otherr ship with id " << sInfo.id << std::endl;
                                //find out ship pos
                                sf::Vector2f p;
                                if (manager->shipPosById(p, sInfo.id))
                                    manager->setTargetForShip(manager->m_shipWithIdWantsANewTarget,
                                                              TTargetType::T_SHIP, sInfo.id, p);
                                // clean up
                                shipWantsTarget = false;
                                manager->m_shipWithIdWantsANewTarget = 0;
                                mouseSprite.setTexture(mouseNormalTexture);

                            }
                            else
                                infoscreen.showShip(sInfo);
                        }
                        else /* clicked ocean */
                        {
                            if(shipWantsTarget)
                            {
                                std::cout << "ship with id " << manager->m_shipWithIdWantsANewTarget <<
                                             "wants target water at (" <<
                                             event.mouseButton.x << "; " <<  event.mouseButton.y <<
                                             ")" << std::endl;
                                manager->setTargetForShip(manager->m_shipWithIdWantsANewTarget,
                                                          TTargetType::T_WATER, 0,
                                                          {1.0f * event.mouseButton.x, 1.0f * event.mouseButton.y});
                                // clean up
                                shipWantsTarget = false;
                                manager->m_shipWithIdWantsANewTarget = 0;
                                mouseSprite.setTexture(mouseNormalTexture);
                            }
                            else
                                infoscreen.showUniverse( event.mouseButton.x, event.mouseButton.y );
                        }
                    }
                }
            }
            else if(event.type == sf::Event::MouseButtonPressed and
                    event.mouseButton.button == sf::Mouse::Right)
            {
                manager->nextRound();
            }
            else if(event.type == sf::Event::KeyPressed)
            {
                window.close();
            }
            else if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }
        window.clear();
        // draw isles
        for(Isle *isle : manager->m_isles)
        {
            window.draw(isle->shape());
        }
        // draw ships on ocean
        for(Ship *ship : manager->m_ships)
        {
            if(ship->positionType() == TShipPosType::S_OCEAN)
                window.draw(ship->shape());
        }


        infoscreen.paint(window);

        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        mouseSprite.setPosition( mousePos.x - 8.0f , mousePos.y - 8.0f );
        window.draw(mouseSprite);

        window.display();
    }
    return 0;
}
