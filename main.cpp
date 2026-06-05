#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <set>
#include <vector>

using namespace geode::prelude;

// We extend PlayLayer to manage the visual pathfinding node and state machine
class $modify(PathfindPlayLayer, PlayLayer) {
    struct Fields {
        cocos2d::CCDrawNode* m_pathDrawNode = nullptr;
        bool m_isJumping = false;
        float m_jumpTimer = 0.0f;
        std::set<GameObject*> m_clickedOrbs;
        float m_lastPlayerX = 0.0f;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontPlay) {
        if (!PlayLayer::init(level, useReplay, dontPlay)) return false;

        // Create the visual pathfinding overlay
        m_fields->m_pathDrawNode = cocos2d::CCDrawNode::create();
        // Add node on top of play layer gameplay elements
        this->addChild(m_fields->m_pathDrawNode, 9999);

        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);

        // Check settings
        bool pathfindEnabled = Mod::get()->getSettingValue<bool>("pathfinding-enabled");
        if (!pathfindEnabled) {
            if (m_fields->m_pathDrawNode) {
                m_fields->m_pathDrawNode->clear();
            }
            return;
        }

        if (!m_player1 || m_player1->m_isDead) return;

        bool pathfindOrbs = Mod::get()->getSettingValue<bool>("pathfind-orbs");
        bool pathfindSpikes = Mod::get()->getSettingValue<bool>("pathfind-spikes");
        bool pathfindVisuals = Mod::get()->getSettingValue<bool>("pathfind-visuals");

        float pX = m_player1->getPositionX();
        float pY = m_player1->getPositionY();
        cocos2d::CCPoint playerPos = m_player1->getPosition();

        if (m_fields->m_pathDrawNode) {
            m_fields->m_pathDrawNode->clear();
        }

        // Maintain the active jumping simulator state machine
        if (m_fields->m_isJumping) {
            m_fields->m_jumpTimer -= dt;
            if (m_fields->m_jumpTimer <= 0.0f) {
                this->releaseButton(0, PlayerButton::Jump);
                m_fields->m_isJumping = false;
            }
        }

        // Clear clicked orbs if we reset or go backwards
        if (pX < m_fields->m_lastPlayerX - 50.0f) {
            m_fields->m_clickedOrbs.clear();
        }
        m_fields->m_lastPlayerX = pX;

        // Search nearby gameplay objects to calculate paths
        if (m_objects) {
            for (int i = 0; i < m_objects->count(); ++i) {
                auto obj = static_cast<GameObject*>(m_objects->objectAtIndex(i));
                if (!obj) continue;

                float oX = obj->getPositionX();
                float oY = obj->getPositionY();
                float distanceX = oX - pX;
                float distanceY = std::abs(oY - pY);

                // Only evaluate objects currently ahead of the player in immediate viewport
                if (distanceX > -30.0f && distanceX < 180.0f) {
                    
                    // ORB DETECTOR (Can be toggled off to discover Swag Routes)
                    bool isOrb = (obj->m_objectType == GameObjectType::RegularRing || 
                                  obj->m_objectType == GameObjectType::GravityRing || 
                                  obj->m_objectType == GameObjectType::Special ||
                                  obj->m_objectType == GameObjectType::CustomRing);
                    
                    if (isOrb && pathfindOrbs) {
                        // Standard trigger boundary check
                        if (distanceX > -15.0f && distanceX < 45.0f && distanceY < 65.0f) {
                            if (m_fields->m_clickedOrbs.find(obj) == m_fields->m_clickedOrbs.end()) {
                                // Jump / click orb
                                this->pushButton(0, PlayerButton::Jump);
                                m_fields->m_isJumping = true;
                                m_fields->m_jumpTimer = 0.08f;
                                m_fields->m_clickedOrbs.insert(obj);

                                // Visual connection path
                                if (pathfindVisuals && m_fields->m_pathDrawNode) {
                                    m_fields->m_pathDrawNode->drawSegment(
                                        playerPos, 
                                        obj->getPosition(), 
                                        3.0f, 
                                        cocos2d::ccc4f(0.0f, 1.0f, 0.5f, 1.0f)
                                    );
                                }
                            }
                        }
                    }

                    // SPIKE / HAZARD DETECTOR (Used to skip spikes automatically)
                    bool isHazard = (obj->m_objectType == GameObjectType::Hazard);
                    if (isHazard && pathfindSpikes) {
                        // Check if hazard is close and we are on the ground
                        if (distanceX > 5.0f && distanceX < 65.0f && distanceY < 50.0f) {
                            if (m_player1->m_isOnGround && !m_fields->m_isJumping) {
                                // Execute path jump over the upcoming spike
                                this->pushButton(0, PlayerButton::Jump);
                                m_fields->m_isJumping = true;
                                m_fields->m_jumpTimer = 0.18f;

                                // Draw indicator vector for hazard path
                                if (pathfindVisuals && m_fields->m_pathDrawNode) {
                                    m_fields->m_pathDrawNode->drawSegment(
                                        playerPos, 
                                        obj->getPosition(), 
                                        2.0f, 
                                        cocos2d::ccc4f(1.0f, 0.2f, 0.2f, 1.0f)
                                    );
                                    m_fields->m_pathDrawNode->drawDot(
                                        obj->getPosition(), 
                                        8.0f, 
                                        cocos2d::ccc4f(1.0f, 0.0f, 0.0f, 0.8f)
                                    );
                                }
                            }
                        }
                    }
                }
            }
        }
    }
};