# Chord Code: briefing visual e funcional para vídeo

Documento de referência para criar vídeos do **Chord Code** (plugin de progressões de acordes da Audio Makers). Reúne conceito, identidade visual, cada elemento da interface, como o plugin funciona, os fluxos de uso, o material de marketing que já existe e sugestões de cenas.

- Versão do produto: **Chord Code Pro 1.2.2** (e **Chord Code Mini 1.1.0**)
- Escrito em 28/10/2026 a partir do código (`edferrazzz/chord-code-mini`, que compartilha o tema visual do Pro), da documentação técnica ([chord-code.md](chord-code.md)), dos screenshots, da landing page v31, dos anúncios e do vídeo v2.
- Todas as imagens estão na pasta de arquivos do projeto. Os links abaixo são relativos a este arquivo (`documentacao/`), então `../chord-code-redesign/depois.png` = `chord-code-redesign/depois.png` na raiz da pasta do projeto.

---

## Sumário

1. [Em uma frase](#1-em-uma-frase)
2. [Conceito e posicionamento](#2-conceito-e-posicionamento)
3. [Identidade visual](#3-identidade-visual)
4. [A interface, peça por peça](#4-a-interface-peça-por-peça)
5. [Telas secundárias (modais)](#5-telas-secundárias-modais)
6. [Movimento e microinterações](#6-movimento-e-microinterações)
7. [Como funciona](#7-como-funciona)
8. [Fluxos de uso (roteiros prontos)](#8-fluxos-de-uso-roteiros-prontos)
9. [Som](#9-som)
10. [Chord Code Mini](#10-chord-code-mini)
11. [Material de marketing que já existe](#11-material-de-marketing-que-já-existe)
12. [Textos e frases prontas](#12-textos-e-frases-prontas)
13. [Sugestões para o vídeo](#13-sugestões-para-o-vídeo)
14. [Inventário de arquivos](#14-inventário-de-arquivos)

---

## 1. Em uma frase

> **Você traz a ideia. O Chord Code encontra os acordes.**

O produtor escolhe **tom, escala, estilo e vibe**, aperta **GERAR** e recebe uma progressão de 4 (ou 8) acordes pronta para ouvir, ajustar e **arrastar como MIDI para a DAW**.

![Interface principal do Chord Code Pro](../chord-code-redesign/depois.png)
*`chord-code-redesign/depois.png`: a tela principal, 1920×1094. É a imagem "oficial" do produto (está na caixa 3D, nos anúncios e na landing).*

---

## 2. Conceito e posicionamento

### O problema
O produtor tem uma ideia (um beat, um clima, uma melodia na cabeça) e **trava na harmonia**. Fica testando acorde no teclado, perde o fluxo, a ideia esfria.

### A promessa
O Chord Code **dá a direção, mas a decisão continua sendo sua**. Não é "o plugin faz a música por você": ele encurta a etapa dos acordes e entrega um ponto de partida editável.

### Os três pilares (usados na landing)
| # | Pilar | O que entra |
|---|---|---|
| 01 | **Encontre a direção** | 9 escalas, 13 estilos, 12 vibes, 4 ou 8 acordes |
| 02 | **Molde a progressão** | Troca manual de acorde, trava por acorde, variações, voicing e inversões, tensão e complexidade |
| 03 | **Dê vida e continue** | Ritmos (Reto, Humanize, Strum, Arpejo), 7 timbres de prévia, histórico, arrastar e salvar MIDI |

### Público
Produtores de beat e música eletrônica/urbana brasileira (trap, funk, R&B, lo-fi, pop), do iniciante que não sabe teoria ao produtor que quer acelerar a experimentação. Linguagem: direta, de produtor pra produtor, em português.

### Tom de voz
Curto, confiante, sem jargão técnico desnecessário. Frases de duas batidas ("Gostou? Trava.", "1 clique. 4 acordes."). Nunca promete que "faz a música sozinho".

### Oferta atual
- **Chord Code Pro: R$37** (de R$97), pagamento único, sem mensalidade, atualizações grátis, Windows e Mac.
- **Chord Code Mini: R$1** (de R$17), porta de entrada que vende o Pro.
- Formatos: VST3 e Standalone (Windows x64); VST3 e AU universais (macOS 10.15+, Intel e Apple Silicon).
- Atenção: alguns anúncios antigos mostram R$47. O preço vigente é **R$37**.

---

## 3. Identidade visual

### 3.1 Clima geral
**Dark premium com calor laranja.** Fundo quase preto, superfícies grafite levemente elevadas, e o laranja aparece como **luz**: brilho difuso atrás da marca, brilho subindo por trás dos cards, glow no botão GERAR e no card que está tocando. A sensação é de hardware de estúdio à noite, com um LED quente aceso. Nada de neon, nada de cores frias; o único acento é o laranja (com o âmbar como ponta clara do gradiente).

### 3.2 Paleta do plugin (tokens do código, `ChordCodeTheme.h`)

**Fundos e superfícies**
| Token | Hex | Uso |
|---|---|---|
| background | `#07080A` | Fundo da janela (base) |
| backgroundTop | `#0E1014` | Topo do gradiente vertical do fundo |
| surface | `#0D0F12` | Base dos cards e painéis |
| surfaceRaised | `#15181D` | Topo do gradiente dos cards/botões (efeito "elevado") |
| surfaceHover | `#1A1E24` | Card/botão com mouse em cima |
| surfaceGroup | `#0B0D10` | Fundo dos grupos (Forma do acorde, Ritmo…) |
| surfaceSelected | `#1A0F08` | Fundo quente de item selecionado |

**Bordas**
| Token | Hex |
|---|---|
| border | `#272B32` |
| borderQuiet | `#1A1D22` |
| borderStrong | `#3A404A` |
| divider | `#1C1F24` |

**Laranja (cor da marca)**
| Token | Hex | Uso |
|---|---|---|
| orange | `#FF5A0A` | Acento principal, glows, seleção, "CODE" do logo |
| orangeBrand | `#FF7A12` | Marca |
| orangeLight | `#FF8C3A` | Texto laranja sobre fundo escuro, ícones ativos |
| amber | `#FFB23D` | Ponta clara do gradiente (GERAR, barras do card tocando) |
| texto sobre laranja | `#160904` | Texto escuro dentro de botões laranja |

**Texto**
| Token | Hex |
|---|---|
| textPrimary | `#F5F6F8` |
| textSecondary | `#8B929C` |
| textQuiet | `#555C66` |
| waveform (barras em repouso) | `#3B414A` |

**Gradiente de acento** (botão GERAR, "CODE" no logo, barras do card ativo, botões primários dos modais):
`#FF5A0A → #FFB23D` (laranja → âmbar), da esquerda para a direita ou de baixo para cima.

**Luz ambiente** (pintada no fundo da janela):
- um brilho radial laranja a 10% de opacidade atrás do logo (canto superior esquerdo);
- um brilho radial laranja a 6% subindo do centro inferior, por trás dos cards.

**Cores de status** (só nos modais): verde "Licenciado" (aprox. `#2FD27A`), vermelho de erro (aprox. `#FF5A4E`).

### 3.3 Paleta da landing page e dos anúncios
| Variável | Hex |
|---|---|
| --bg | `#050606` |
| --panel | `#0B0D0D` |
| --line | `#292D2C` |
| --text | `#F7F7F4` |
| --muted | `#A8ACA8` |
| --orange | `#FF5A0A` |

O anúncio "claro-notebook" é a única peça em **fundo claro** (off-white quente com blur), usada para contraste no feed.

### 3.4 Tipografia
| Onde | Fonte | Detalhes |
|---|---|---|
| Plugin | **Inter** (Light, Regular, Bold, embutida) | Rótulos em CAIXA ALTA, Bold, tracking largo (~8%), 13 px em `#8B929C`. Valores dos dropdowns em Bold caixa alta. Nome do acorde enorme em Bold. |
| Landing e anúncios: títulos | **Space Grotesk** (500–700) | Títulos grandes, "CHORD CODE" da caixa 3D, preço |
| Landing: texto | **Manrope** (400–800) | Corpo, botões |

Padrão tipográfico dos títulos de marketing: primeira linha branca, segunda linha **laranja** ("Você traz a ideia. / *Ele encontra os acordes.*"; "Crie progressões / *em segundos*").

### 3.5 Formas
- Raio da janela: **18 px**; cards e painéis: **14 px**; controles (dropdowns, botões): **10 px**. Pílulas (tags, versão) totalmente arredondadas.
- Cards e botões têm **gradiente vertical sutil** (topo mais claro) e um **reflexo branco de 7%** no topo, como plástico/vidro fosco.
- Bordas finas de 1 px; quando ativo, a borda vira laranja e ganha glow.
- Divisória horizontal sob o cabeçalho: linha que some nas pontas (gradiente transparente → cinza → transparente).

### 3.6 Logo e ícone
- **Ícone**: quadrado arredondado laranja (gradiente laranja/âmbar) com **barras verticais de forma de onda** em escuro (lembra um equalizador / waveform). Arquivos: `Assets/Brand/` no repo do Mini (`chord-code-mini-icon-1024.png`, `.svg`, `.ico`).
- **Wordmark**: "CHORD" em branco + "CODE" em laranja (gradiente laranja → âmbar), Bold, caixa alta. No Mini, vem seguido de "MINI" em cinza claro, fino e espaçado.
- Assinatura: **AUDIO MAKERS** em caixa alta espaçada (no Mini: "AUDIO" cinza, "MAKERS" laranja).
- Variação do vídeo final: "Chord Code **Pro**" em caixa mista, "Pro" em laranja.

---

## 4. A interface, peça por peça

Janela do Pro em 1120×638 (escalável). Uma tela só, tudo à vista, nada escondido em menu. Leitura de cima para baixo: **configurar → ver/ouvir → moldar → agir**.

![Interface anotável](../chord-code-redesign/depois.png)

### 4.1 Cabeçalho
- **Esquerda**: ícone + wordmark CHORD CODE.
- **Direita**: pílula de status com ponto laranja ("C MENOR NATURAL · 4 ACORDES · 120 BPM"), pílula de versão ("v1.2.1"/"v1.2.2") e **engrenagem** (configurações). Quando há atualização, uma **bolinha âmbar pulsa** na engrenagem.

### 4.2 Linha 1: direção harmônica (4 dropdowns)
`TOM` · `ESCALA` · `ESTILO` · `VIBE`
Cada um é uma caixa grafite com rótulo pequeno acima e um botão quadrado de seta (chevron) à direita.

### 4.3 Linha 2: caráter
`COMPLEXIDADE` · `TENSÃO` · `TIMBRE`

![Dropdown aberto](../chord-code-redesign/dropdown.png)
*`chord-code-redesign/dropdown.png`: lista aberta; item atual destacado em laranja translúcido.*

### 4.4 Os cards de acorde (o coração visual)
Quatro cards lado a lado (até oito com Continuar). Cada card mostra:
- **Número** da posição em etiqueta pequena (01, 02, 03, 04); laranja no card selecionado.
- **Ícones** no canto superior direito: ↻ variação (troca só este acorde), 🔒 trava, ▶ tocar (aparece no card selecionado/tocando).
- **Nome do acorde** enorme, branco, Bold (Cmadd9, Gm, G#add9, Dsus2…).
- **Grau** em pílula laranja (i, v, VI, II; minúsculo = menor, maiúsculo = maior) seguido das **notas** separadas por pontos ("C · D · D# · G").
- **Mini waveform**: 9 barras verticais arredondadas, simétricas, mais altas no centro. Em repouso são cinza (`#3B414A`).

**Estados do card**
| Estado | Visual |
|---|---|
| Repouso | Grafite, borda cinza fina, barras cinza |
| Hover | Fundo um pouco mais claro, borda mais forte, sobe 3 px |
| Selecionado | Borda laranja, glow laranja leve (35%) |
| **Tocando** | Borda laranja forte com glow, **luz laranja subindo do pé do card**, barras viram gradiente âmbar → laranja e **pulsam com o som** |
| Travado | Cadeado fechado laranja |

![Animação dos cards](../chord-code-redesign/cards-animacao.gif)
*`chord-code-redesign/cards-animacao.gif`: o destaque passando de card em card durante a reprodução. Ótima referência de movimento.*

**Card "Continuar"** (5º espaço): borda tracejada, círculo com "+", texto "CONTINUAR / +4 ACORDES". Leva a progressão de 4 para 8 acordes mantendo os 4 primeiros.

### 4.5 Painel de forma e ritmo
Três grupos em caixas mais escuras:
- **FORMA DO ACORDE** (segmentado): PADRÃO · FECHADO · ABERTO · INVERTER
- **RITMO** (segmentado): RETO · HUMANIZE · STRUM · ARPEJO
- **DURAÇÃO** (dropdown 1/4, 2/4, 4/4) e **VOLUME** (slider laranja com bolinha branca de borda laranja, valor 0–100)

O segmento ativo é uma pílula **laranja sólida** com texto escuro; os outros são texto cinza.

### 4.6 Barra de ações (rodapé)
`PARAR/REPRODUZIR` · `VARIAÇÃO` · `ALEATÓRIO` · **`GERAR`** · `HISTÓRICO` · `ARRASTAR MIDI` · `SALVAR MIDI`

- **GERAR** é o maior elemento da tela: botão largo com **gradiente laranja → âmbar**, texto escuro espaçado ("G E R A R") e **glow laranja** forte ao redor. É o herói de qualquer cena.
- Os demais são botões grafite com ícone de linha à esquerda e rótulo em caixa alta. O ícone de ALEATÓRIO (setas cruzadas) é laranja.

---

## 5. Telas secundárias (modais)

Todos os modais abrem sobre a interface **desfocada e escurecida**, num cartão grafite com leve gradiente quente no topo, raio grande e ícone laranja à esquerda do título.

### 5.1 Seletor de acordes ("Trocar acorde")
Clique num card → abre o seletor com duas abas.

**Campo harmônico**: os acordes da escala agrupados por função (pílula com o grau em laranja + nome: MEDIANTE, SUBDOMINANTE, DOMINANTE, SUBTÔNICA…). O acorde atual fica com **borda laranja**. Rodapé: "Passe o mouse para ouvir · clique para trocar".

![Seletor: campo harmônico](../chord-code-redesign/seletor-acordes.png)

**Todos os acordes**: grade de 12 fundamentais × 15 qualidades (maior, m, 7, maj7, m7, dim, m7b5, dim7, aug, sus2, sus4, add9, 9, maj9, m9), inclusive fora do tom. Na landing: "180 acordes para escolher".

![Seletor: todos os acordes](../chord-code-redesign/v4_chords_all.png)

### 5.2 Configurações
Título "CONFIGURAÇÕES / Chord Code" + pílula de versão laranja. Seções: **TEMPO** (Sincronizar BPM com toggle laranja, "projeto: 120 BPM", grade de BPMs 80–160), **LICENÇA** ("● Licenciado" em verde, Trocar), **ATUALIZAÇÕES** (Verificar atualizações), crédito do piano.

![Configurações](../chord-code-redesign/v4_settings.png)

### 5.3 Ativação por e-mail
"LICENÇA / Ative o Chord Code": campo "E-MAIL DA COMPRA" e botão laranja **Ativar**. Na imagem, estado de erro em vermelho.

![Ativação](../chord-code-redesign/v4_license_error.png)

### 5.4 Nova versão disponível
Ícone de download laranja, "v1.2.2 → v1.2.3" em pílulas, notas da versão, botões "Agora não" e **Baixar atualização**.

![Atualização disponível](../chord-code-redesign/v4_update_available.png)

### 5.5 Instalador (Windows)
Janelas pequenas (640×500) no mesmo tema escuro: opções → progresso → concluído.

| Opções | Progresso | Concluído |
|---|---|---|
| ![](../chord-code-redesign/instalador-options.png) | ![](../chord-code-redesign/instalador-progress.png) | ![](../chord-code-redesign/instalador-done.png) |

---

## 6. Movimento e microinterações

Como o plugin se mexe (tirado do código); use como referência de animação no vídeo:

- **Easing suave**, sem "bounce": hover aproxima o alvo a ~30% por frame, ativação a ~22% (curva exponencial de saída, tipo ease-out).
- **Hover**: o card sobe 3 px, clareia levemente e a borda fica mais visível.
- **Tocando**: o glow laranja acende ao redor do card, uma luz quente sobe da base e as 9 barras viram gradiente âmbar/laranja, **pulsando** com a reprodução. Ao passar para o próximo acorde, o destaque "caminha" de card em card (ver o GIF).
- **GERAR**: sempre com glow; no clique, os nomes dos acordes trocam (ótimo momento para corte/whoosh).
- **Bolinha de atualização**: pulsa em âmbar.
- **Modais**: fundo desfoca e escurece; o cartão aparece por cima.
- **Arrastar MIDI**: o usuário arrasta a partir do botão e solta o arquivo `.mid` na DAW (piano roll).

---

## 7. Como funciona

### 7.1 Controles e todas as opções
| Controle | Opções |
|---|---|
| **Tom** | C, C#, D, D#, E, F, F#, G, G#, A, A#, B |
| **Escala (9)** | Maior, Menor Natural, Menor Harmônica, Menor Melódica, Dórica, Frígia, Lídia, Mixolídia, Frígia Dominante |
| **Estilo (13 + Padrão)** | Pop, Trap, Hip-Hop, R&B, EDM, House, Lo-Fi, Phonk, Indie, Funk Brasileiro, Latin, Cinematic, Jazz |
| **Vibe (12 + Padrão)** | Melancólico, Emocional, Atmosférico, Divertido, Sombrio, Romântico, Nostálgico, Esperançoso, Misterioso, Tenso, Épico, Agressivo |
| **Complexidade** | Automático, Simples, Colorido, Rico |
| **Tensão** | Padrão, Suave, Média, Alta |
| **Timbre (7)** | Piano, Rhodes (Piano Elétrico), Órgão, Pad Atmosférico, Saw, Violino, Flauta |
| **Forma do acorde** | Padrão, Fechado, Aberto, Inverter |
| **Ritmo** | Reto, Humanize, Strum, Arpejo |
| **Duração** | 1, 2 ou 4 tempos por acorde |
| **Volume** | 0 a 100 (velocity do MIDI) |
| **BPM** | Segue a DAW ou fixo (40 a 240) |

Os números de marketing: **9 escalas, 13 estilos, 12 vibes, 7 timbres, até 8 acordes, 180 acordes no seletor**. (O vídeo v2 e o pop-up do Mini contam o "Padrão": 14 estilos / 13 vibes. Escolha um padrão e mantenha.)

### 7.2 Ações
- **Gerar**: progressão nova com as configurações atuais.
- **Variação**: outra progressão, respeitando os acordes travados.
- **Aleatório**: sorteia tom, escala, estilo, vibe, complexidade e tensão e gera.
- **Continuar**: de 4 para 8 acordes (e "Voltar a 4").
- **Trava por acorde**: o acorde travado não muda nas próximas gerações.
- **Variação por acorde (↻)**: troca só aquele acorde por outro que combine.
- **Trocar acorde (clique)**: abre o seletor (campo harmônico ou todos os acordes); passar o mouse toca, clicar troca.
- **Reproduzir/Parar**, ou clique no card para ouvir um acorde.
- **Histórico**: as últimas 10 gerações.
- **Arrastar MIDI** direto para a DAW, ou **Salvar MIDI** (`.mid`).

### 7.3 O que acontece por trás (versão para narração)
1. O plugin monta as notas da escala a partir do tom.
2. Cada **estilo** e cada **vibe** dão peso a certos graus (Pop puxa I, IV, V, vi; Lo-Fi e R&B puxam ii, IV, vi; Melancólico puxa vi e IV).
3. Ele escreve **420 progressões candidatas** e dá nota para cada uma: caminhos que soam bem (V → I, ii → V), semelhança com progressões clássicas do estilo, movimento suave entre os acordes.
4. Escolhe a melhor e calcula o **voicing** com a menor distância entre um acorde e o próximo (condução de vozes natural).
5. **Não repete**: lembra as últimas 64 progressões e descarta as repetidas. "Cada GERAR traz algo novo."

Frase-resumo: *"Ele pensa como um produtor do gênero e evita repetir."*

### 7.4 Saída MIDI
MIDI de uma faixa, com o ritmo escolhido (Reto, Humanize com pequenos atrasos e variação de força, Strum como dedilhado, Arpejo uma nota por vez), 100% editável na DAW.

---

## 8. Fluxos de uso (roteiros prontos)

### Fluxo principal: "Da ideia ao MIDI" (5 passos da landing)
**01 ESCOLHER → 02 GERAR → 03 OUVIR → 04 AJUSTAR → 05 ARRASTAR MIDI**

1. Escolhe **tom** e **escala** (ex.: A menor natural).
2. Escolhe **estilo** e **vibe** (ex.: Lo-Fi + Nostálgico).
3. Aperta **GERAR**. Quatro acordes aparecem.
4. Aperta **Reproduzir**: o destaque laranja caminha de card em card com o som.
5. Gostou de três? **Trava** os três e aperta **Variação** só no outro.
6. Ou **clica** num card e escolhe qualquer acorde no seletor.
7. Muda a **forma** (Aberto) e o **ritmo** (Strum).
8. Aperta **Continuar** para ir a 8 acordes.
9. **Arrasta o MIDI** para a DAW.

### Fluxo "Quer mais cor?"
Complexidade de Simples → **Rico**: os acordes ganham 7ª, 9ª, 11ª, sus e add9 (ex.: Am → Am9, F → Fmaj7).

### Fluxo "Gostou? Trava."
Travar 2 acordes → GERAR → só os outros mudam.

### Fluxo "Qualquer acorde, até fora do tom"
Clique no card → aba **Todos os acordes** → passa o mouse ouvindo → clica.

### Fluxo do Mini → Pro
Abre o Mini → gera → clica em **UPGRADE PARA O PRO** → pop-up "Componha sem limites" → "Quero o Chord Code Pro".

### Primeira abertura do Pro
Ativação com e-mail da compra → "Licenciado" → funciona offline.

---

## 9. Som

### Timbres de prévia
Amostras de cada timbre em `chord-code-redesign/timbres/`:
[piano.mp3](../chord-code-redesign/timbres/piano.mp3) · [rhodes.mp3](../chord-code-redesign/timbres/rhodes.mp3) · [organ.mp3](../chord-code-redesign/timbres/organ.mp3) · [pad.mp3](../chord-code-redesign/timbres/pad.mp3) · [saw.mp3](../chord-code-redesign/timbres/saw.mp3) · [violino.mp3](../chord-code-redesign/timbres/violino.mp3) · [flauta.mp3](../chord-code-redesign/timbres/flauta.mp3)

- **Piano** (padrão): Salamander Grand Piano (crédito obrigatório quando aparecer: "Piano: Salamander Grand Piano, Alexander Holm (CC BY 3.0)").
- **Rhodes, Saw, Órgão**: sintetizados. **Pad, Violino, Flauta**: samples.
- Todos normalizados em torno de −20 dB RMS (volume parecido entre eles).

### Referência de mix do vídeo anterior
Vídeo v2: narração em PT (voz "Elvis" da ElevenLabs), música de fundo com ducking, um trecho de ~11 s só com o **som real do plugin** (com selo "SOM REAL DO PLUGIN" na tela), SFX de clique/whoosh, masterizado em −14 LUFS.

---

## 10. Chord Code Mini

Versão reduzida (R$1) com a mesma estética, feita para vender o Pro.

![Chord Code Mini 1.1](../chord-code-mini/mini-1.1-tela.png)
*`chord-code-mini/mini-1.1-tela.png`*

- Wordmark "CHORD CODE **MINI**" e botão contornado laranja **UPGRADE PARA O PRO** (com seta para cima) no canto.
- Controles: Tom, Escala (Maior/Menor), Vibe (Atmosférica, Melancólica, Romântica, Alegre), Voicing.
- 4 cards só com o ícone de variação (sem trava, sem Continuar).
- Ações: Reproduzir, Variação, **GERAR**, Arrastar MIDI, Salvar MIDI.
- Só tríades, sétimas e dominantes; só piano.

**Pop-up de upgrade** ("CHORD CODE PRO / Componha sem limites"): imagens do Pro à esquerda e lista de vantagens com checks laranja à direita.

![Upgrade do Mini](../chord-code-mini/mini-1.1-upgrade.png)

---

## 11. Material de marketing que já existe

### Caixa 3D (packshot)
Caixa escura com lateral laranja vertical escrita "CHORD CODE", frente com logo, subtítulo "Progressões de acordes prontas para sua DAW", screenshot da interface, pílulas VST3 · AU · Win·Mac e "AUDIO MAKERS". Fundo preto com brilho laranja.

![Caixa 3D](../chord-code-page/loja/chordcode-box-3d-1920x1080.png)

Versões: `chord-code-page/loja/chordcode-box-3d-1920x1080.png`, `-1200x1200.png`, `-transparente.png` (sem fundo, ideal para compor no vídeo).

### Landing page (v31)
Hero: tags de formato, "FEITO PARA QUEM PRODUZ", título "Você traz a ideia. **O CHORD CODE encontra os acordes.**", e a interface num **tablet inclinado em perspectiva** com borda luminosa laranja/roxa.

![Hero da landing](../chord-code-page/preview-v30-hero.png)
![Oferta da landing](../chord-code-page/preview-v30-oferta.png)

Página inteira: `chord-code-page/preview-v29-desktop.png` (960×6542).

### Anúncios (feed 1080×1080 / 1080×1350 e story 1080×1920)
| Conceito | Visual | Arquivos |
|---|---|---|
| **Você traz a ideia** | Título branco + laranja, 3 cards de acorde flutuando inclinados, caixa 3D, pílulas de números, CTA laranja | `chord-code-anuncios/voce-traz-a-ideia-*.png` |
| **Travou na harmonia** | Gancho de dor | `chord-code-anuncios/travou-na-harmonia-*.png` |
| **Claro notebook** | Fundo claro off-white, waveform + logo, "Crie progressões **em segundos**", interface num notebook, botão "Saiba mais →" | `chord-code-anuncios/claro-notebook-*.png` |

![Anúncio Você traz a ideia](../chord-code-anuncios/voce-traz-a-ideia-feed-1080x1350.png)
![Anúncio claro notebook](../chord-code-anuncios/claro-notebook-feed-1080x1350.png)

(Os anúncios mostram R$47; atualizar para R$37 se reaproveitar.)

### Vídeos
| Arquivo | Duração | Conteúdo |
|---|---|---|
| `chord-code-video/chord-code-pro-video-v2.mp4` | 65 s, 1080p30 | Vídeo de venda do Pro (versão atual, usada na landing) |
| `chord-code-video/chord-code-pro-video-v1.mp4` | 65 s | Mesma edição, música 3 dB mais alta |
| `chord-code-video/tutorial-instalacao.mp4` | 75 s | Tutorial de instalação |

**Estrutura do vídeo v2** (para não repetir ou para evoluir):
1. Gancho no piano roll vazio: "Beat **pronto.**" (falta harmonia).
2. "1 clique. **4 acordes.**" Interface gerando.
3. "Qualquer tom." Dropdown de tom aberto, número gigante "12".
4. "Escolhe a vibe." Números gigantes **14 ESTILOS / 13 VIBES** e "LO-FI + NOSTÁLGICO".
5. "Quer mais **cor?**" Complexidade Rica: aparecem nonas, sus, add9.
6. "Gostou? **Trava.**" Cadeados laranja.
7. "Qualquer acorde. **Até fora do tom.**" Seletor aberto.
8. Voicing e ritmo: Humanize, Strum, Arpejo.
9. Interface tocando com selo "SOM REAL DO PLUGIN" e o destaque caminhando pelos cards.
10. "Do zero a uma ideia completa em menos de um minuto."
11. Card final: ícone, "Chord Code **Pro**", "Pare de travar nos acordes.", pílulas (Até 8 acordes · 14 estilos · 13 vibes · 9 escalas · 7 timbres · MIDI na DAW), botão "GARANTA O SEU AGORA", `chordcodepro.audiomakers.club`, "VST3 · AU · WINDOWS · MACOS".

Estilo de legenda: títulos curtos centralizados no topo, parte em branco e parte em laranja; legenda da narração pequena e branca embaixo.

---

## 12. Textos e frases prontas

**Títulos / ganchos**
- Você traz a ideia. **O Chord Code encontra os acordes.**
- Pare de travar nos acordes.
- Crie progressões **em segundos**.
- Da ideia ao MIDI.
- 1 clique. **4 acordes.**
- Gostou? **Trava.**
- Quer mais **cor?**
- Qualquer acorde. **Até fora do tom.**
- Menos tentativa. **Mais música.**
- Gerar não significa perder o controle.
- O plugin dá a direção. A decisão continua sendo sua.
- Ele encontra a direção. Você continua no controle.
- Cada GERAR traz algo novo.
- A progressão não termina no Chord Code.
- Tudo à vista. Nada escondido em menu.
- Pensa como um produtor do gênero.
- Não soa como bloco de MIDI.

**Verbos do fluxo** (sequência da landing): **GERE** uma base. **ESCOLHA** um acorde específico. **TRAVE** o que funcionou. **VARIE** somente o que quiser. **CONTINUE** na sua DAW.

**CTAs**: QUERO O CHORD CODE · QUERO O CHORD CODE POR R$37 · GARANTA O SEU AGORA · QUERO LEVAR PARA MINHA DAW · Saiba mais →

**Selos de confiança**: Compra segura · Download imediato · Windows e Mac · Pagamento único, sem mensalidade · Atualizações grátis

**Links**: Pro `chordcodepro.audiomakers.club` · Mini `chordcode.audiomakers.club`

---

## 13. Sugestões para o vídeo

### Linguagem visual recomendada
- **Fundo**: preto `#050606`/`#07080A` com brilhos laranja difusos (radial, baixa opacidade), igual à janela do plugin. Opcional: grade sutil, como nos anúncios.
- **Interface sempre real** (screenshots ou captura em 2x), nunca redesenhada. Em close, isolar os cards, o GERAR e o seletor.
- **Perspectiva**: a interface em tablet/janela inclinada em 3D com borda luminosa (como no hero da landing) funciona bem para abertura e final.
- **Cards flutuando** inclinados (como no anúncio "Você traz a ideia") para cenas de "quatro acordes aparecem".
- **Números gigantes** em Space Grotesk com o rótulo pequeno embaixo (12 tons, 9 escalas, 13 estilos, 12 vibes, 7 timbres, 180 acordes, 8 acordes).
- **Laranja só como luz e destaque**: glow do GERAR, card tocando, palavra-chave do título. Se tudo for laranja, nada se destaca.

### Cenas-chave que vendem
1. **O clique no GERAR** com glow e os nomes dos acordes trocando (repetir 2–3 vezes rápido mostra o "nunca repete").
2. **O destaque caminhando pelos cards** com o som real (sincronizar o glow com cada acorde).
3. **Travar e variar**: cadeado fechando em laranja, só um card mudando.
4. **Seletor**: mouse passando pela grade, botões acendendo.
5. **Arrastar MIDI** do botão para o piano roll de uma DAW, as notas aparecendo.
6. **Packshot final**: caixa 3D transparente sobre fundo escuro com brilho laranja + preço R$37 + CTA.

### Formatos
- Horizontal 1920×1080 para landing/YouTube; vertical 1080×1920 para Reels/TikTok/Stories (no vertical, mostrar a interface cortada nos cards e no GERAR, já que a tela inteira fica pequena).

### Cuidados
- Preço vigente: **R$37** (de R$97).
- Coerência de números: escolher "13 estilos / 12 vibes" (sem contar Padrão, como na landing) ou "14 / 13" (como no vídeo v2), e manter o mesmo em todo o vídeo.
- Os screenshots mostram "v1.2.1" na pílula do cabeçalho; não é problema, mas evite close nessa pílula.
- Ao mostrar o piano, o crédito do Salamander já aparece nas configurações; não precisa estar no vídeo, mas não remova de telas que o mostram.
- Não dizer que o plugin "faz a música sozinho".

---

## 14. Inventário de arquivos

Caminhos relativos à raiz da pasta de arquivos do projeto.

**Interface do Pro**
| Arquivo | O que é | Tamanho |
|---|---|---|
| `chord-code-redesign/depois.png` | Tela principal (oficial) | 1920×1094 |
| `chord-code-redesign/antes.png` | Interface antiga, antes do redesign (não usar como produto atual) | 1920×1094 |
| `chord-code-redesign/dropdown.png` | Dropdown aberto | 1920×1094 |
| `chord-code-redesign/seletor-acordes.png` | Seletor: campo harmônico | 1920×1094 |
| `chord-code-redesign/v4_chords_all.png` | Seletor: todos os acordes | 2240×1276 |
| `chord-code-redesign/v4_settings.png` | Configurações | 2240×1276 |
| `chord-code-redesign/v4_license_error.png` | Ativação com erro | 2240×1276 |
| `chord-code-redesign/v4_update_available.png` | Nova versão disponível | 2240×1276 |
| `chord-code-redesign/cards-animacao.gif` | Animação dos cards tocando | 940×175 |
| `chord-code-redesign/instalador-{options,progress,done}.png` | Instalador Windows | 640×500 |
| `chord-code-redesign/timbres/*.mp3` | Amostras dos 7 timbres | — |

**Mini**
| Arquivo | O que é |
|---|---|
| `chord-code-mini/mini-1.1-tela.png` | Tela do Mini 1.1 (2240×956) |
| `chord-code-mini/mini-1.1-upgrade.png` | Pop-up de upgrade 1.1 |
| `chord-code-mini/mini-tela.png`, `mini-upgrade.png` | Versão 1.0 (antiga) |

**Marketing**
| Arquivo | O que é |
|---|---|
| `chord-code-page/loja/chordcode-box-3d-{1920x1080,1200x1200,transparente}.png` | Caixa 3D |
| `chord-code-page/preview-v30-hero.png`, `preview-v30-oferta.png` | Landing: hero e oferta |
| `chord-code-page/preview-v29-desktop.png` | Landing inteira |
| `chord-code-page/CHORD-CODE-Cloudflare-video-v31.zip` | Código da landing atual (CSS, fontes, assets) |
| `chord-code-anuncios/*.png` | 8 anúncios (3 conceitos × feed/story) |
| `chord-code-video/chord-code-pro-video-v2.mp4` | Vídeo de venda atual |
| `chord-code-video/tutorial-instalacao.mp4` | Tutorial de instalação |

**Marca (no repositório `edferrazzz/chord-code-mini`)**
| Arquivo | O que é |
|---|---|
| `Assets/Brand/chord-code-mini-icon-1024.png`, `.svg`, `.ico` | Ícone |
| `Assets/Upgrade/pro-interface.jpg`, `pro-picker.jpg`, `pro-eight.jpg` | Imagens do Pro usadas no pop-up |
| `Source/ChordCodeTheme.h` | Tokens de cor e raios |

**Documentação técnica completa**: [documentacao/chord-code.md](chord-code.md)
