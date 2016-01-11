/* This file is part of WaterWorld.
 * License GPL Version 3, see file "LICENSE" for details
 * Author Eike Lange
 */

#ifndef MANAGER_H
#define MANAGER_H

#include <cstdlib>  // weak random number generator, @fixme: replace with <random>
#include <iostream>
#include <functional>   // std::function
#include <list>
#include "assert.h"

#include "isle.h"
#include "ship.h"


/* Postoffice' job is just to support a friend function. This friend function
 * is allowed to access any member of Postoffice, but not "Manager",
 * wich is a derived class.
 */
struct Postoffice
{
    enum TShipMessageEnum { M_DELETE_SHIP, M_SET_TARGET, M_PATROL } ;

    friend void ship_message_function(Postoffice * receiver, TShipMessageEnum msg, unsigned int shipId);

    // this is what is received by derived classes
    virtual void receive_ship_message(TShipMessageEnum msg, unsigned int shipId) = 0;

};

/* this is called from whoever wants to send ship messages to Manager */
void ship_message_function(Postoffice * receiver, Postoffice::TShipMessageEnum msg, unsigned int shipId)
{
    receiver->receive_ship_message(msg, shipId);
}


typedef std::vector<ShipInfo> TShipInfos;


struct Manager : Postoffice
{
    Manager(const unsigned int inWidth,
            const unsigned int inHeight,
            const unsigned int inNumIsles,
            const unsigned int inNumEnemies)
    {
        assert( inNumIsles > inNumEnemies );
        m_lastInsertedId = 0;

        m_shipWithIdWantsANewTarget = 0;

        for(unsigned int i = 0; i < inNumIsles; i++)
        {
            m_lastInsertedId++;
            bool too_close = false;
            float x, y;
            do
            {
                // random pos
                x = rand() % (inWidth  - 20) + 10.0f;
                y = rand() % (inHeight - 20) + 10.0f;
                too_close = false;

                // 2 isles must not be too close to each other
                // @fixme: distance is hard coded
                for(unsigned int j = 0; j < i; j++)
                {
                    Isle *tmp = m_isles.at(j);
                    sf::Vector2f tmpPos = tmp->pos();
                    float dist2 = (tmpPos.x - x) * (tmpPos.x - x) + (tmpPos.y - y) * (tmpPos.y - y);
                    if(dist2 < 400.0f)
                    {
                        too_close = true;
                        break;
                    }

                }
            } while(too_close);

            // create new isle at "random" pos
            Isle *isle = new Isle(m_lastInsertedId, 0, {x, y}, sf::Color::White);
            m_isles.push_back(isle);
        }

        for(unsigned int i = 0; i < inNumEnemies; i++)
        {
            Isle *tmp = m_isles.at(i);
            tmp->setOwner(i+2, sf::Color::Red);
        }

        // Isle for human player
        Isle *tmp = m_isles.at(inNumEnemies);
        tmp->setOwner(1, sf::Color::Yellow);

        // @fixme: just testing
        createShipAtIsle(tmp->id());
        createShipAtIsle(tmp->id());
    }


    void createShipAtIsle(unsigned int inId)
    {
        for(Isle *tmp : m_isles)
        {
            if(tmp->id() == inId)
            {
                createShipAtIsle(tmp);
                break;
            }
        }
    }


    void createShipAtIsle(Isle* &refIsle)
    {
        m_lastInsertedId++;
        unsigned int owner = refIsle->owner();
        sf::Vector2f iPos = refIsle->pos();
        sf::Color color = refIsle->color();
        Ship *s = new Ship(m_lastInsertedId, owner, iPos, color, TShipPosType::S_ONISLE, refIsle->id());
        m_ships.push_back(s);
    }


    /**
     * @brief isleInfoByPos
     * @param outInfo valid IsleInfo returned to caller, if (inX, inY) is point on isle, else undefined
     * @param inX mouse X pos
     * @param inY mouse Y pos
     * @return true, if (inX, inY) are a point on an isle
     */
    bool isleInfoByPos(IsleInfo & outInfo, const int inX, const int inY)
    {
        for(Isle *isle : m_isles)
        {
            if(isle->pointInIsle(inX, inY))
            {
                outInfo = isle->info();
                return true;
            }
        }
        return false;
    }


    /**
     * @brief islePosById
     * @param outPos pos of isle with given id
     * @param inIsleId id of isle to retrive pos
     * @return true, if inIsleId is valid
     */
    bool islePosById(sf::Vector2f  & outPos, const unsigned int inIsleId)
    {
        for(Isle *isle : m_isles)
        {
            if(isle->id() == inIsleId)
            {
                outPos = isle->pos();
                return true;
            }
        }
        return false;
    }


    /**
     * @brief shipPosById
     * @param outPos pos of ship with given id
     * @param inShipId id of ship to retrive pos
     * @return true, if inShipId is valid
     */
    bool shipPosById(sf::Vector2f  & outPos, const unsigned int inShipId)
    {
        for(Ship *ship : m_ships)
        {
            if(ship->id() == inShipId)
            {
                outPos = ship->pos();
                return true;
            }
        }
        return false;
    }

    /**
     * @brief shipInfoByPos: returns ShipInfo if coordinates match a ship on the ocean
     * @param outInfo valid ShipInfo returned to caller, if (inX, inY) is point on ship, else undefined
     * @param inX mouse X pos
     * @param inY mouse Y pos
     * @return true, if (inX, inY) are a point on an ship
     */
    bool shipInfoByPos(ShipInfo & outInfo, const int inX, const int inY)
    {
        for(Ship *ship: m_ships)
        {
            if(ship->pointInShip(inX, inY) and (ship->postionType() == TShipPosType::S_OCEAN))
            {
                outInfo = ship->info();
                return true;
            }
        }
        return false;
    }


    bool shipInfosByIsleInfo(TShipInfos & outShipInfos, IsleInfo inInfo)
    {
        ShipInfo sInfo;
        if(inInfo.owner != 1) /* just for human players. Enemie ships are not our business */
            return false;
        outShipInfos.clear();
        for(Ship *ship: m_ships)
        {
            if(ship->postionType() != TShipPosType::S_OCEAN)
            {
                sInfo = ship->info();
                if( (sInfo.owner == 1 /* Human Player */) and
                    (inInfo.id == sInfo.isleId /* same isle */))
                {
                    outShipInfos.push_back(sInfo);
                    std::cout << "shipInfosByIsleInfo() -> append ship" << sInfo.id << std::endl;

                }
            }
        }
        if(outShipInfos.empty())
            return false;
        return true;
    }


    /**
     * @brief receive_ship_message - receive a command about a ship from Infoscreen
     * @param inMsg - the command (delete ship, ...)
     * @param inShipId - unique id of the ship
     */
    void receive_ship_message(Postoffice::TShipMessageEnum inMsg, unsigned int inShipId)
    {
        std::cout << "Manager: Receive a message for ship " << inShipId << std::endl;
        Ship *actionShip = 0;
        for(Ship *s : m_ships)
        {
            if(s->id() == inShipId)
            {
                actionShip = s;
                break;
            }
        }
        switch(inMsg)
        {
        case M_DELETE_SHIP:
            // with nextround(), the ship gets deleted
            actionShip->setPositionType(TShipPosType::S_TRASH);
            break;
        case M_SET_TARGET:
            if(m_shipWithIdWantsANewTarget == inShipId)
                m_shipWithIdWantsANewTarget = 0;
            else
                this->m_shipWithIdWantsANewTarget = inShipId;
            std::cout << " --> ship wants new target " <<  this->m_shipWithIdWantsANewTarget << std::endl;
            // lot of things to do
            //actionShip.setPositionType(TShipPosType::S_OCEAN);
            break;
        case M_PATROL:
            actionShip->setPositionType(TShipPosType::S_PATRUILLE);
            break;
        }
    }


    void setTargetForShip(const unsigned int inShipId,  const TTargetType inTargetType,
                          const unsigned int inTargetId, const sf::Vector2f inTargetPos)
    {
        for(Ship *s : m_ships)
        {
            if(s->id() == inShipId)
            {
                switch(inTargetType)
                {
                    case TTargetType::T_ISLE:
                        s->setTargetIsle(inTargetId, inTargetPos);
                        break;
                    case TTargetType::T_SHIP:
                        s->setTargetShip(inTargetId, inTargetPos);
                        break;
                    case TTargetType::T_WATER:
                        s->setTargetWater(inTargetPos);
                        break;
                }
                break;
            }
        }
    }




    /* *********************************** */
    /* Data begins here                    */
    /* *********************************** */

    std::vector<Isle*> m_isles;     // all isles
    std::list<Ship*> m_ships;       // all ships
    unsigned int m_lastInsertedId;  // every ship or isle gets a unique id. this stores the next one.
    unsigned int m_shipWithIdWantsANewTarget;   // > 0, if a ship (with this id) wants a new target
};

#endif // MANAGER_H
