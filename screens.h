/* This file is part of WaterWorld.
 * License GPL Version 3, see file "LICENSE" for details
 * Author Eike Lange
 */

#ifndef SCREENS_H
#define SCREENS_H

#include <SFML/Graphics.hpp>
#include <iostream>
#include <algorithm>        // std::min
#include <functional>
#include <string>

#include <isle.h>
#include <ship.h>
#include <manager.h>


struct ShipListList
{
    static const int MAX_ROWS = 10;

    ShipListList()
    {
        m_itemRowHeight = 15.0f;
        m_numLabelsToShow = 0;
        m_highlightedLabel = -1;

        if(!m_font.loadFromFile("FreeMono.ttf"))
        {
            std::cout << "canot load font" << std::endl;
        }
        m_actionButtonTexture.loadFromFile("ShipsOnIsleActionButtons.png");
        m_statusButtonTexture.loadFromFile("ShipsOnIsleStatusButtons.png");

        m_actionButtons.setTexture(m_actionButtonTexture);
        m_statusButtons.setTexture(m_statusButtonTexture);

        for(int i = 0; i < MAX_ROWS; i++)
        {
            m_statusLabel[i].setTexture(m_statusButtonTexture);
            m_statusLabel[i].setTextureRect({33, 0, 11, 11});

            m_nameLabels[i].setFont(m_font);
            m_nameLabels[i].setCharacterSize(12);
            m_nameLabels[i].setColor(sf::Color::White);
            m_nameLabels[i].setString("XXXX");

            m_damageLabels[i].setFont(m_font);
            m_damageLabels[i].setCharacterSize(12);
            m_damageLabels[i].setColor(sf::Color::Green);
            m_damageLabels[i].setString("100%");

            m_techLabels[i].setFont(m_font);
            m_techLabels[i].setCharacterSize(12);
            m_techLabels[i].setColor(sf::Color::White);
            m_techLabels[i].setString("TECH");
        }
    }


    void init(const unsigned int inXOffset, const unsigned int inYOffset,
              const unsigned int inWidth, unsigned int inHeight,
              std::function<void(Postoffice::TShipMessageEnum, unsigned int)> & inShipMessageFunction)
    {
        m_width = inWidth;
        m_height = inHeight;
        m_XOffset = inXOffset;
        m_YOffset = inYOffset;

        m_shipMessageFunction = inShipMessageFunction;  // func to send commands to manager

        if(!m_renderTexture.create(m_width, m_height))
        {
            std::cout << "cannot create render texture" << std::endl;
        }

        m_targetSprite.setPosition(m_XOffset, m_YOffset);
        m_actionButtons.setPosition(0, m_height - 20.0f);
        m_statusButtons.setPosition(m_width - 50.0f, m_height-20.0f);

        for(int i = 0; i < MAX_ROWS; i++)
        {
            m_statusLabel[i].setPosition({0.0f, m_itemRowHeight * i + 2.0f});
            m_nameLabels[i].setPosition({15.0f, m_itemRowHeight * i});
            m_damageLabels[i].setPosition({100.0f, m_itemRowHeight * i});
            m_techLabels[i].setPosition({100.0f + 40.0f, m_itemRowHeight * i});
        }
    }


    bool testClickOnLabels(int & outLabelNum, /* inX,*/ const unsigned int inY)
    {
        sf::FloatRect f0 = m_nameLabels[0].getGlobalBounds();
        if(inY > f0.top)
        {
            sf::FloatRect fn = m_nameLabels[MAX_ROWS-1].getGlobalBounds();
            if(inY < fn.top + fn.height)
            {
                outLabelNum = (inY -f0.top) / m_itemRowHeight;
                return true;
            }
            return false;
        }
        return false;
    }


    void processActionDeleteShip()
    {
        ShipInfo si = m_shipInfos.at(m_highlightedLabel);
        m_shipMessageFunction(Postoffice::M_DELETE_SHIP, si.id);
    }


    void processActionSetTargetForShip()
    {
        ShipInfo si = m_shipInfos.at(m_highlightedLabel);
        m_shipMessageFunction(Postoffice::M_SET_TARGET, si.id);
    }


    void processActionSetPatrol()
    {
        ShipInfo si = m_shipInfos.at(m_highlightedLabel);
        m_shipMessageFunction(Postoffice::M_PATROL, si.id);
    }


    void processClick(const int inX, const int inY)
    {
        unsigned int px = inX - m_XOffset;
        unsigned int py = inY - m_YOffset;

        if(px < m_width and py < m_height)
        {
            int labelNum = -1;
            if((py > m_height - 20) and (px < 60) and (m_highlightedLabel > -1))
            {
                // click on action button?
                int actionButtonNum = px/20;
                std::cout << "click on action button with num: " << actionButtonNum << std::endl;
                switch(actionButtonNum)
                {
                    case 0:
                        processActionDeleteShip();
                        break;
                    case 1:
                        processActionSetTargetForShip();
                        break;
                    case 2:
                        processActionSetPatrol();
                        break;
                }

            }
            else if(testClickOnLabels(labelNum, py))
            {
                // click on ship label?
                if(labelNum >= m_numLabelsToShow) return;
                std::cout << "click on label with num: " << labelNum << std::endl;

                if(m_highlightedLabel == -1)
                {
                    m_highlightedLabel = labelNum;
                    m_nameLabels[m_highlightedLabel].setColor(sf::Color::Yellow);
                    m_nameLabels[m_highlightedLabel].setStyle(sf::Text::Bold);
                }
                else
                {
                    m_nameLabels[m_highlightedLabel].setColor(sf::Color::White);
                    m_nameLabels[m_highlightedLabel].setStyle(sf::Text::Regular);
                    if(m_highlightedLabel == labelNum)
                    {
                        m_highlightedLabel = -1;
                    }
                    else
                    {
                        m_highlightedLabel = labelNum;
                        m_nameLabels[m_highlightedLabel].setColor(sf::Color::Yellow);
                        m_nameLabels[m_highlightedLabel].setStyle(sf::Text::Bold);
                    }
                }
            }
        }
    }


    void paint(sf::RenderWindow & refWindow)
    {
        m_renderTexture.clear();
        m_renderTexture.draw(m_actionButtons);
        m_renderTexture.draw(m_statusButtons);

        for(int i = 0; i < m_numLabelsToShow; i++)
        {
            m_renderTexture.draw(m_statusLabel[i]);
            m_renderTexture.draw(m_nameLabels[i]);
            m_renderTexture.draw(m_damageLabels[i]);
            m_renderTexture.draw(m_techLabels[i]);
        }

        m_renderTexture.display();
        m_targetSprite.setTexture(m_renderTexture.getTexture());
        refWindow.draw(m_targetSprite);
    }


    void setShipInfos(TShipInfos inShipInfos)
    {
        m_shipInfos = inShipInfos;

        // start with fresh labels
        for(int i = 0; i < MAX_ROWS; i++)
        {
            m_nameLabels[i].setColor(sf::Color::White);
            m_nameLabels[i].setStyle(sf::Text::Regular);
        }

        m_numLabelsToShow = std::min(MAX_ROWS, (int) m_shipInfos.size());
        for(int i = 0; i < m_numLabelsToShow; i++)
        {
            ShipInfo si = m_shipInfos.at(i);
            std::string shipName = "Ship " + std::to_string(si.id);
            m_nameLabels[i].setString(shipName);
            switch(si.posType)
            {
                case ShipPositionEnum::S_PATRUILLE:
                    m_statusLabel[i].setTextureRect({0, 0, 11, 11});
                    break;
                case ShipPositionEnum::S_TRASH:
                    m_statusLabel[i].setTextureRect({11, 0, 11, 11});
                    break;
                default:
                    if(si.hasTarget)
                        m_statusLabel[i].setTextureRect({22, 0, 11, 11});
                    else
                        m_statusLabel[i].setTextureRect({33, 0, 11, 11});
                    break;
            }
        }

    }


    /* *********************************** */
    /* Data begins here                    */
    /* *********************************** */

    unsigned int m_width;
    unsigned int m_height;
    float m_XOffset;
    float m_YOffset;

    float m_itemRowHeight;

    // render target
    sf::RenderTexture m_renderTexture;
    sf::Sprite m_targetSprite;


    sf::Font m_font;

    // Sprites and textures
    sf::Texture m_statusButtonTexture;
    sf::Texture m_actionButtonTexture;
    sf::Sprite m_actionButtons;
    sf::Sprite m_statusButtons;

    // Labels
    sf::Sprite m_statusLabel[MAX_ROWS]; // icon shows current status
    sf::Text m_nameLabels[MAX_ROWS];    // Name of the Ship
    sf::Text m_damageLabels[MAX_ROWS];  // Damage in Percent
    sf::Text m_techLabels[MAX_ROWS];    // Techlevel

    // Displaying ships
    TShipInfos m_shipInfos;
    int m_numLabelsToShow;              // 0..MAX_ROWS-1, min(NumberOfShips, MAX_ROWS)
    int m_highlightedLabel;             // number of highlighted label or -1
    std::function<void(Postoffice::TShipMessageEnum, unsigned int)>
        m_shipMessageFunction;  // message function to Manager
};


struct InfoScreen
{
    enum TScreenEnum {I_NOTHING, I_ISLE, I_HUMAN_ISLE, I_WATER, I_SHIP, I_HUMAN_SHIP};
    static const int NUM_ITEMS_IN_SHIPLIST = 10;


    /**
     * @brief InfoScreen - Shows up an infoscreen on the right side of the screen.
     * @param inXPos - right pos (top is not needed, as we start from 0)
     * @param inWidth - width of the screen
     * @param inHeight - height of the screen
     * @param inMessageFunction - communication ShipListList --> Manager, just forward to ShipListList.
     */
    InfoScreen(const unsigned int inXPos, const unsigned int inWidth, unsigned int inHeight,
               std::function<void(Postoffice::TShipMessageEnum, unsigned int)> inMessageFunction)
    {
        m_width = inWidth * 1.0f;
        m_height = inHeight * 1.0f;
        m_xPos = inXPos * 1.0f;
        m_screenType = I_NOTHING;

        m_rectangle.setSize({m_width, m_height});
        m_rectangle.setFillColor(sf::Color(255, 0, 255, 128));
        m_rectangle.setPosition({m_xPos, 0.0f});
        if( !m_font.loadFromFile("FreeMono.ttf") )
        {
            // error
            std::cout << "InfoScreen: ERR loading font" << std::endl;
        }
        m_titleText.setFont(m_font);
        m_titleText.setCharacterSize(14);
        m_titleText.setColor(sf::Color::Yellow);

        m_posText.setFont(m_font);
        m_posText.setCharacterSize(14);
        m_posText.setColor(sf::Color::Yellow);
        m_posText.setPosition({m_xPos + 10.0f, 30.0f});

        m_idOwnerText.setFont(m_font);
        m_idOwnerText.setCharacterSize(14);
        m_idOwnerText.setColor(sf::Color::Yellow);
        m_idOwnerText.setPosition({m_xPos + 10.0f, 30.0f});

        /* items for human player's isle */
        m_sectionFont.loadFromFile("FreeMono.ttf");
        m_shipListTitle.setFont(m_sectionFont);
        m_shipListTitle.setCharacterSize(10);
        m_shipListTitle.setColor(sf::Color::Green);
        m_shipListTitle.setString("Shiplist");
        m_shipListTitle.setPosition({m_xPos + 10.0f, 50.0f});
        m_listItemHeight = 15.0f;
        m_shipListList.init(inXPos + 2, 100, inWidth-4, inHeight - 200, inMessageFunction);

        /* items for human ship */
        m_targetTypeLabel.setFont(m_font);
        m_targetTypeLabel.setCharacterSize(12);
        m_targetTypeLabel.setColor(sf::Color::Yellow);
        m_targetTypeLabel.setPosition({m_xPos + 10.0f, 30.0f});
        m_targetTypeLabel.setString("Target:");

        m_targetTypeText.setFont(m_font);
        m_targetTypeText.setCharacterSize(12);
        m_targetTypeText.setColor(sf::Color::Yellow);
        m_targetTypeText.setPosition({m_xPos + 10.0f, 45.0f});

    }


    bool pointInfoscreen(const int inX)
    {
        return inX > m_xPos;
    }


    void processInfoscreenClick(const int inX, const int inY)
    {
        if(m_screenType == I_HUMAN_ISLE)
        {
            m_shipListList.processClick(inX, inY);
        }
    }


    void paint(sf::RenderWindow & refWindow)
    {
        refWindow.draw(m_rectangle);

        // display title centered
        sf::FloatRect titleRect = m_titleText.getLocalBounds();
        float w = m_xPos + (m_width - titleRect.width) / 2;
        m_titleText.setPosition({w, 10});
        refWindow.draw(m_titleText);

        // display extra info -- depending on screen
        switch(m_screenType)
        {
            case TScreenEnum::I_WATER:
                refWindow.draw(m_posText);
                break;
            case I_ISLE:
            case I_SHIP:   // OK at the moment
                refWindow.draw(m_idOwnerText);
                break;
            case I_HUMAN_ISLE:
                refWindow.draw(m_idOwnerText);
                refWindow.draw(m_shipListTitle);
                m_shipListList.paint(refWindow);
                break;
            case I_HUMAN_SHIP:
                refWindow.draw(m_targetTypeLabel);
                refWindow.draw(m_targetTypeText);
                break;
            default:
                break;
        }
    }


    void showUniverse(const int inX, const int inY)
    {
        m_rectangle.setFillColor(sf::Color(20, 20, 255, 128));
        m_titleText.setString("Water");
        std::string s = "Pos: (" + std::to_string(inX) + ", " + std::to_string(inY) + ")";
        m_posText.setString(s);
        m_screenType = I_WATER;
    }


    void showIsle(const IsleInfo inIsleInfo)
    {
        m_rectangle.setFillColor(sf::Color(20, 20, 255, 128));
        m_titleText.setString("Isle");
        std::string s = "ID: " + std::to_string(inIsleInfo.id) + ", Owner: " + std::to_string(inIsleInfo.owner);
        m_idOwnerText.setString(s);
        m_screenType = I_ISLE;
    }


    void showHumanIsle(const IsleInfo inIsleInfo, const TShipInfos inShipInfos)
    {
        m_rectangle.setFillColor(sf::Color(20, 20, 255, 128));
        m_titleText.setString("Human Isle");
        std::string s = "ID: " + std::to_string(inIsleInfo.id) + ", Owner: Player";
        m_idOwnerText.setString(s);
        m_shipListList.setShipInfos(inShipInfos);
        m_screenType = I_HUMAN_ISLE;
    }


    void showShip(const ShipInfo inShipInfo)
    {
        m_rectangle.setFillColor(sf::Color(20, 20, 255, 128));
        m_titleText.setString("Ship");
        std::string s = "ID: " + std::to_string(inShipInfo.id) + ", Owner: " + std::to_string(inShipInfo.owner);
        m_idOwnerText.setString(s);
        m_screenType = I_SHIP;
    }


    void showHumanShip(const ShipInfo inShipInfo, const Target inTarget)
    {
        m_rectangle.setFillColor(sf::Color(20, 20, 255, 128));
        m_titleText.setString("Human Isle");
        std::string s = "ID: " + std::to_string(inShipInfo.id) + ", Owner: Player";
        m_idOwnerText.setString(s);

        if(inTarget.validTarget)
        {
            switch(inTarget.tType)
            {
                case Target::T_ISLE:
                    m_targetTypeText.setString("Isle");
                    break;
                case Target::T_SHIP:
                    m_targetTypeText.setString("Ship");
                    break;
                case Target::T_WATER:
                    m_targetTypeText.setString("Water");
                    break;
            }
        }
        else
            m_targetTypeText.setString("no target");
        m_screenType = I_HUMAN_SHIP;
    }



    /* *********************************** */
    /* Data begins here                    */
    /* *********************************** */



    sf::RectangleShape m_rectangle; // infoscreen is just a rectangle
    float m_xPos;               // position (relative to main screen, see main.cpp)
    float m_width, m_height;    // width, height of this infoscreen
    TScreenEnum m_screenType;   // current active screen

    sf::Font m_font;            // font for text items
    sf::Text m_titleText;       // Title of screen
    sf::Text m_posText;         // Universe position
    sf::Text m_idOwnerText;     // Isle and Ship infos (id and owner)

    /* human isle screen */
    float m_listItemHeight;     // height of an item in List. Must be >= fontSize
    sf::Font m_sectionFont;     // font for sections on screen
    sf::Text m_shipListTitle;   // title above shiplist
    ShipListList m_shipListList;

    /* human ship screen */
    sf::Text m_targetTypeLabel;
    sf::Text m_targetTypeText;


};


#endif // SCREENS_H
