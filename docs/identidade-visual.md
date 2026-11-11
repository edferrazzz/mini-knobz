# Mini Knobz: identidade visual

## Regras que não mudam

- As 5 janelas são **idênticas** na estrutura; só a cor de destaque muda.
- Fundo **quase preto**, sem textura. Nada de texto explicativo na janela: a explicação fica no popup **i**.
- Rodapé sempre com **"by audiomakers"** e versão.
- Tamanho de design 300x480 (mostrado a 125%).

## Cor de cada plugin

| Plugin | Cor | Hex |
|---|---|---|
| BITE | laranja-vermelho | `#FF5A36` |
| PHAT | rosa | `#FF4D9A` |
| KICKSIDE | verde-limão | `#B6F23A` |
| WIDE | ciano | `#3DD6FF` |
| GASP | violeta | `#A98BFF` |

Nas peças do pacote, os 5 aparecem juntos sempre nessa ordem (BITE, PHAT, KICKSIDE, WIDE, GASP) e as 5 bolinhas coloridas viram o símbolo do pacote ("●●●●● 5 PLUGINS · 1 KNOB CADA").

## Cores de base da interface

| Token | Hex |
|---|---|
| Fundo (topo → base, gradiente vertical) | `#121318` → `#08080B` |
| Poço do knob / áreas fundas | `#060608`, borda `#1B1C23` |
| Chips e botões | `#111217`, borda `#262833` |
| Texto | `#ECEDF3` |
| Texto suave | `#C9CBD8` |
| Texto apagado | `#8B8EA0` |
| Desligado | `#4A4C58` |
| OK / mono seguro | `#3DE08A` |
| Aviso | `#FFD23F` |

## Fontes

- **Space Grotesk Bold**: nome do plugin (BITE, GASP...) e títulos com letras espaçadas.
- **Space Grotesk Medium**: opções e rótulos (TAPE, DEPTH, 1/4).
- **Inter Bold / Regular**: números do knob, textos de ajuda, rodapé.

Todas em `Assets/Fonts/` (SIL Open Font License).

## Anatomia da janela (de cima para baixo)

1. Topo: ícone de power + "knobz", botões **i** (ajuda) e engrenagem (configurações).
2. Nome do plugin grande, na cor dele.
3. Barra de opções (TAPE/TUBE/DRIVE/CLIP etc.); a escolhida fica com caixa e brilho na cor.
4. **Knob grande**: anel de traços que acendem na cor conforme gira, número no centro (45, 40, 70, 50, 1/2), rótulo embaixo (DRIVE, AMOUNT, DEPTH, WIDTH, SIZE).
5. Linha extra: slider MIX / MONO BASS, seletor de tempo (KICKSIDE) ou forma de onda + play + botão **WAV** arrastável (GASP).
6. Rodapé: bypass (power), "by audiomakers", versão.

No KICKSIDE as opções são **desenhos de curva** (não texto), gerados pela mesma função que processa o áudio.

## Peças prontas

Veja [README.md](README.md) para a lista de imagens em `img/`. Story e capas usam as janelas em leque com glow suave nas cores, título "MINI KNOBZ" branco e "Um knob. Som pronto." em gradiente rosa → violeta → ciano.
