#include "misc.h"

void misc::init(c_player_controller *player)
{
    if (!g.b_walk || !g.b_strafer)
        return;
    c_movement_controller *movement{};
    c_player_translation_parameters *params{};
    c_jump_parameters *jump{};
    c_translation_data *data{};

    if (player)
    {
        movement = player->m_pMovement;

        if (movement)
        {

            params = movement->m_pTranslationParams;

            if (params)
            {

                jump = params->m_pJumpParameters;

                if (jump)
                {

                    data = movement->m_pTranslationData;

                    if (data)
                    {

                        if (g.b_walk)
                        {
                            params->m_fMecanimDirectionChangeSpeed = 9999;
                            params->m_fMoveDirectionChangeSpeed = 9999;
                        }

                        if (g.b_strafer)
                        {
                            data->a = Vector3{};
                            jump->jump_speed = 6.8f;
                        }
                    }
                }
            }
        }
    }
}