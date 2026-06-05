#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

// IDs das Orbs para o bot saber quando clicar no ar
enum GDOrbs {
    OrbAmarela = 35,
    OrbRosa = 140,
    OrbVermelha = 1332,
    OrbAzul = 84,
    OrbVerde = 1022,
    DashOrbVerde = 1704,
    DashOrbRosa = 1751
};

class $modify(MyPlayLayer, PlayLayer) {
    bool segurandoDash = false;
    float cronometroDash = 0.0f;

    void update(float dt) {
        PlayLayer::update(dt);

        if (!this->m_player1) return;

        auto player = this->m_player1;
        cocos2d::CCPoint posPlayer = player->m_position;
        
        // Pega o tamanho físico (Hitbox) do próprio jogador
        cocos2d::CCRect hitboxPlayer = player->boundingBox();

        // Identificando os Modos de Jogo principais para a física
        bool isCube  = !player->m_isShip && !player->m_isBall && !player->m_isBird && !player->m_isDart && !player->m_isRobot && !player->m_isSpider && !player->m_isSwing;
        bool isShip  = player->m_isShip;
        bool isBall  = player->m_isBall;
        bool isUFO   = player->m_isBird;
        bool isWave  = player->m_isDart;
        bool isRobot = player->m_isRobot;
        bool isSpider = player->m_isSpider;
        bool isSwing = player->m_isSwing;

        // Segurança da Dash Orb
        if (segurandoDash) {
            cronometroDash += dt;
            if (cronometroDash > 1.5f || player->m_isDead) {
                player->releaseButton(PlayerButton::Jump);
                segurandoDash = false;
            }
        }

        // 🔍 ENGINE GEOMÉTRICA DE VARREDURA (Lendo Hitboxes)
        auto objetos = this->m_objects;
        CCObject* obj = nullptr;
        
        CCARRAY_FOREACH(objetos, obj) {
            auto gameObj = static_cast<GameObject*>(obj);
            
            // Pega a Hitbox Real do Objeto no frame atual (resolve os espinhos triplos colados!)
            cocos2d::CCRect hitboxObjeto = gameObj->boundingBox();
            
            // Calcula a distância real em pixels entre a frente do player e o início do objeto
            float distX = hitboxObjeto.getMinX() - hitboxPlayer.getMaxX();
            float distY = hitboxObjeto.getMinY() - hitboxPlayer.getMaxY();

            // Só processa objetos que estão logo à frente (até 160 pixels)
            if (distX > -10.0f && distX < 160.0f) {
                
                // 🔺 TRATAMENTO DE ACIDENTES (Qualquer tipo de Espinho Individual ou Agrupado)
                if (gameObj->m_objectType == GameObjectType::Hazard) {
                    
                    // Se for Cubo, Robô, Bola ou Aranha e o espinho (ou os 3 colados) estiver chegando perto:
                    if ((isCube || isRobot || isBall || isSpider) && player->m_isGrounded) {
                        // Ajusta o tempo de reação baseado na distância da frente do obstáculo
                        if (distX < 55.0f) { 
                            player->pushButton(PlayerButton::Jump);
                            log::info("🤖 AI: Perigo detectado via Hitbox a {}px! Pulando...", distX);
                        }
                    }
                    // Desvio para modos aéreos (Nave, Onda, Swing)
                    else if ((isShip || isWave || isSwing) && abs(distY) < 35.0f && distX < 65.0f) {
                        player->pushButton(PlayerButton::Jump);
                    }
                }

                // 🧱 TRATAMENTO DE BLOCOS SÓLIDOS E PAREDES
                if (gameObj->m_objectType == GameObjectType::Solid) {
                    // Se o bloco estiver exatamente na altura da cara do cubo (Parede vertical)
                    float alturaCubo = hitboxPlayer.getMaxY() - hitboxPlayer.getMinY();
                    bool paredeNaEspera = (hitboxObjeto.getMinY() < player->m_position.y + 10.0f) && (hitboxObjeto.getMaxY() > player->m_position.y);

                    if ((isCube || isRobot) && paredeNaEspera && distX < 45.0f && player->m_isGrounded) {
                        player->pushButton(PlayerButton::Jump); // Pula para subir no bloco em vez de estourar nele
                        log::info("🤖 AI: Parede de Bloco detectada! Subindo na plataforma.");
                    }
                }

                // 🟡 LEITURA DIRETA DAS ORBS (Mantido por ID de ativação)
                int id = gameObj->m_objectID;
                float centroX = abs(gameObj->m_position.x - posPlayer.x);
                float centroY = abs(gameObj->m_position.y - posPlayer.y);

                if (centroX < 28.0f && centroY < 28.0f) {
                    // Orbs Normais
                    if (id == GDOrbs::OrbAmarela || id == GDOrbs::OrbRosa || id == GDOrbs::OrbVermelha || 
                        id == GDOrbs::OrbAzul || id == GDOrbs::OrbVerde) {
                        
                        player->releaseButton(PlayerButton::Jump);
                        player->pushButton(PlayerButton::Jump);
                    }

                    // Dash Orbs Reais (Segura o botão de forma contínua)
                    if ((id == GDOrbs::DashOrbVerde || id == GDOrbs::DashOrbRosa) && !segurandoDash) {
                        segurandoDash = true;
                        cronometroDash = 0.0f;
                        player->pushButton(PlayerButton::Jump);
                        log::info("🤖 AI: HOLD acionado na Dash Orb Real!");
                    }
                }

                // Solta o botão automaticamente ao passar pelo perigo
                if (!isWave && !isShip && !isSwing && !segurandoDash && distX > 75.0f) {
                    player->releaseButton(PlayerButton::Jump);
                }
            }
        }
    }
};