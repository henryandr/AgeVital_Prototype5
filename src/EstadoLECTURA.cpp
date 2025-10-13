#ifndef ESTADO_LECTURA_CPP
#define ESTADO_LECTURA_CPP

#include "State.h"
#include "StateMachine.h"

class EstadoLECTURA : public State
{
public:
    void onEnter() override
    {
        Serial.println("===Entrando en Estado LECTURA===");
        statemachine->flags.lectura = true;
        statemachine->flags.inicio = false;
        statemachine->flags.envio = false;
    }

    void execute() override
    {
        unsigned long now = millis();

        if (statemachine->flags.dev)
        {
            Serial.println("Cambiando a Estado DESARROLLADOR desde LECTURA");
            statemachine->ChangeState(new EstadoDESARROLLADOR());
            return;
        }
        // Control de inactividad
        if (statemachine->isDisplayOn && (now - statemachine->clocks.ultima_interaccion > statemachine->settings.TIEMPO_INACTIVIDAD))
        {
            // Apagar pantalla
            display.clearDisplay();
            display.display();
            statemachine->isDisplayOn = false;
            Serial.println("Pantalla apagada por inactividad");
        }

        // Actualizar pantalla si es necesario
        if (statemachine->needsUpdate && statemachine->isDisplayOn)
        {
            updateDisplay();
            statemachine->needsUpdate = false;
        }

        // Lectura periódica de sensores
        if (now - statemachine->clocks.tiempo_lectura >= statemachine->settings.INTERVALO_LECTURA)
        {
            Serial.println("Estado: LECTURA");
            readSensors();
            if (statemachine->isDisplayOn)
            {
                updateDisplay();
            }
            statemachine->clocks.tiempo_lectura = now;
        }
        // Verificar si es tiempo de envío
        if (statemachine->flags.envio_programado && (long)(now - statemachine->clocks.proximo_envio) >= 0)
        {
            statemachine->ChangeState(new EstadoENVIO());
        }
    }

    void onExit() override
    {
        Serial.println("===Saliendo de Estado LECTURA===");
        statemachine->flags.lectura = false;
    }

    const char *getName() override
    {
        return "LECTURA";
    }
};

#endif