#ifndef ESTADO_INICIO_CPP
#define ESTADO_INICIO_CPP

#include "State.h"
#include "StateMachine.h"

class EstadoINICIO : public State
{

private:
    bool firstRun = true;

public:
    void onEnter() override
    {
        Serial.println("===Entrando en Estado INICIO===");
        statemachine->flags.inicio = true;
        firstRun = true;
    }

    void execute() override
    {
        if (!firstRun)
            return; // Solo ejecutar una vez

        // Verificar si se debe cambiar al estado DESARROLLADOR
        if (statemachine->flags.dev)
        {
            Serial.println("Cambiando a Estado DESARROLLADOR desde INICIO");
            statemachine->ChangeState(new EstadoDESARROLLADOR());
            return;
        }
        unsigned long now = millis();

        Serial.println("Estado: INICIO");
        statemachine->flags.inicio = false;
        statemachine->flags.proximo_envio = now + statemachine->settings.INTERVALO_ENVIO;
        statemachine->flags.envio_programado = true;
        statemachine->ChangeState(new EstadoLECTURA());
        firstRun = false; // Asegurar que no se vuelva a ejecutar
    }

    void onExit() override
    {
        Serial.println("===Saliendo de Estado INICIO===");
        statemachine->flags.inicio = false;
        firstRun = false;
    }

    const char *getName() override
    {
        return "INICIO";
    }
};

#endif
