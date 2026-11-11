# Mini Knobz: documentação

Tudo o que é preciso para falar, vender e fazer vídeo do Mini Knobz sem abrir o código.

| Arquivo | O que tem |
|---|---|
| [produto.md](produto.md) | O que é, para quem, preço, cada plugin (o que faz, opções, quando usar), argumentos de venda e FAQ |
| [identidade-visual.md](identidade-visual.md) | Cores, fontes, anatomia da janela, regras visuais que não podem mudar |
| [video/roteiro-reverse-reverb.md](video/roteiro-reverse-reverb.md) | Roteiro de vídeo curto (9:16) começando pelo GASP + prompt pronto para ferramenta de vídeo |
| [img/](img/) | Telas dos plugins, popup de ajuda, capas da loja, story, logo |
| [audio/](audio/) | Antes/depois de cada plugin (mesmos áudios da landing) |

## Imagens

| Arquivo | Uso |
|---|---|
| `img/bite.png`, `phat.png`, `kickside.png`, `wide.png`, `gasp.png` | Janela de cada plugin sozinha, 2x, fundo do próprio plugin |
| `img/gasp-vazio.png` | GASP antes de receber áudio ("Arraste o áudio aqui") |
| `img/kickside-ajuda.png` | Popup "i" aberto (exemplo do texto de ajuda) |
| `img/telas-todas.png` | As 5 janelas lado a lado |
| `img/popup-ajuda-e-gasp-vazio.png` | Popup de ajuda + GASP vazio, lado a lado |
| `img/capa-1200x1200.png`, `capa-1920x1080.png`, `capa-transparente.png` | Capas da loja (caixa 3D) |
| `img/story-1080x1920.png` | Story de venda (5 janelas em leque, "Os 5 por R$17") |
| `img/logo-audiomakers.png` | Marca Audio Makers |

As telas por plugin saem do próprio código: `KNOBZ_SNAPSHOT_DIR=pasta xvfb-run ./knobz_tests` (veja o README da raiz).

## Áudios

`original.mp3` é o loop seco usado para BITE, PHAT, KICKSIDE e WIDE. `gasp-original.mp3` é o acorde seco; `gasp-smooth/stutter/rise.mp3` são o swell do GASP terminando no acorde (4 s, 100 BPM). Regenerar: `KNOBZ_DEMO_DIR=pasta ./knobz_tests`.
