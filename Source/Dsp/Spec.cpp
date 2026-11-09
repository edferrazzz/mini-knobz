#include "Spec.h"

namespace knobz {

namespace {
const std::array<Spec, 5>& all() {
    static const std::array<Spec, 5> specs {{
        { PluginId::bite, "BITE", "Knobz Bite", 0xffff5a36, "DRIVE", 45.0f,
          { "TAPE", "TUBE", "DRIVE", "CLIP" }, RowType::slider, "MIX",
          "Satura\xc3\xa7\xc3\xa3o que esquenta e d\xc3\xa1 mordida sem subir o volume.",
          { { "KNOB", "Mais drive. O volume de sa\xc3\xad" "da se ajusta sozinho." },
            { "TAPE", "Fita: suave e quente, arredonda os agudos." },
            { "TUBE", "V\xc3\xa1lvula: harm\xc3\xb4nicos pares, encorpa." },
            { "DRIVE", "Pedal: mais agressivo e presente." },
            { "CLIP", "Segura os picos. \xc3\x93timo em 808 e bateria." },
            { "MIX", "Mistura com o som original." } } },
        { PluginId::phat, "PHAT", "Knobz Phat", 0xffff4d9a, "AMOUNT", 40.0f,
          { "WARM", "PUNCH", "CRUSH", "AIR" }, RowType::slider, "MIX",
          "Deixa qualquer som mais gordo e mais na frente.",
          { { "KNOB", "Quanto de peso e presen\xc3\xa7" "a." },
            { "WARM", "Corpo nos graves, arredonda." },
            { "PUNCH", "Ataque e peso para bateria e 808." },
            { "CRUSH", "O mais agressivo: tudo na cara." },
            { "AIR", "Brilho e presen\xc3\xa7" "a nos agudos." },
            { "MIX", "Mistura com o som original." } } },
        { PluginId::kickside, "KICKSIDE", "Knobz Kickside", 0xffb6f23a, "DEPTH", 70.0f,
          { "CLASSIC", "SMOOTH", "HARD", "LONG", "CHOP" }, RowType::tempo, "TEMPO",
          "O bombeado do sidechain no tempo da DAW, sem rotear o bumbo.",
          { { "KNOB", "Quanto o volume afunda a cada batida." },
            { "CURVAS", "O desenho \xc3\xa9 o volume: cai em cada batida e volta." },
            { "CLASSIC", "O pump padr\xc3\xa3o de house." },
            { "SMOOTH", "Respiro suave, sem corte." },
            { "HARD", "Corte seco e volta r\xc3\xa1pida." },
            { "LONG", "Afunda e volta devagar. Bom em pads." },
            { "CHOP", "Corte r\xc3\xadtmico, quase um gate." },
            { "TEMPO", "De 1/1 a 1/16. 1/4 \xc3\xa9 o cl\xc3\xa1ssico." } } },
        { PluginId::wide, "WIDE", "Knobz Wide", 0xff3dd6ff, "WIDTH", 50.0f,
          { "CLEAN", "DOUBLE", "SPREAD" }, RowType::slider, "MONO BASS",
          "Abre o est\xc3\xa9reo de pads, vozes e leads sem sumir no mono.",
          { { "KNOB", "Do natural ao bem largo." },
            { "CLEAN", "Abre o que j\xc3\xa1 \xc3\xa9 est\xc3\xa9reo." },
            { "DOUBLE", "Cria est\xc3\xa9reo at\xc3\xa9 de um som mono." },
            { "SPREAD", "Abre mais os agudos, bem brilhante." },
            { "MONO BASS", "Abaixo dessa frequ\xc3\xaancia tudo fica no centro." },
            { "MONO", "O que o WIDE adiciona some na soma mono: nunca cancela." } } },
        { PluginId::gasp, "GASP", "Knobz Gasp", 0xffa98bff, "SIZE", 50.0f,
          { "SMOOTH", "STUTTER", "TAPE", "RISE" }, RowType::wave, "",
          "Arraste um trecho, o GASP cria o reverse reverb e devolve o WAV pronto.",
          { { "\xc3\x81UDIO", "Arraste um WAV da DAW ou clique para abrir." },
            { "KNOB", "Tamanho do swell: de 1/8 a 2 compassos, no BPM da DAW." },
            { "STUTTER", "O swell picotado no tempo." },
            { "TAPE", "Puxada de fita, com pitch." },
            { "RISE", "O swell sobe de tom at\xc3\xa9 a nota." },
            { "WAV", "Arraste para a DAW. O WAV termina onde o som come\xc3\xa7" "a." } } },
    }};
    return specs;
}
} // namespace

const Spec& spec(PluginId id) { return all()[(size_t) id]; }

} // namespace knobz
