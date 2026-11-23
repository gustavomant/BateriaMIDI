# Bateria MIDI com Arduino

Projeto de bateria eletrônica utilizando Arduino, sensores piezoelétricos, JACK e Hydrogen.

## Requisitos

- Lubuntu 24.04 LTS
- Arduino com o sketch `BateriaMIDI.ino`
- Sensores piezoelétricos
- PipeWire/JACK

## Instalação das dependências

```bash
sudo apt update
sudo apt install -y $(grep -vE '^\s*(#|$)' dependencias.txt)
```

## Compilação do bridge MIDI

```bash
make -C bridge
```

## Inicialização

Conecte o Arduino e identifique a porta serial:

```bash
ls /dev/ttyUSB* /dev/ttyACM* 2>/dev/null
```

Inicie todo o ambiente:

```bash
./iniciar_bateria.sh /dev/ttyUSB0
```

Se o Arduino estiver em `/dev/ttyACM0`, utilize:

```bash
./iniciar_bateria.sh /dev/ttyACM0
```

O script inicia o Hydrogen, o bridge MIDI e o QPWGraph, além de tentar conectar automaticamente as portas MIDI.

## Mapeamento dos pads

No Hydrogen, selecione **Opções → Modo de entrada → Drumkit**. Em
**Opções → Preferências → Sistema MIDI**, desmarque **Usar nota de saída como
nota de entrada** e confirme em **OK**. A tabela de eventos pode ficar vazia.
Assim, as notas MIDI seguem a ordem dos instrumentos no editor de padrões,
independentemente do kit carregado. O campo **Saída MIDI → Nota** no editor de
instrumentos controla a saída MIDI e não precisa ser alterado.

| Porta Arduino | Instrumento no kit atual | Nota MIDI |
|---------------|--------------------------|-----------|
| A0 | 1º da lista | 36 |
| A1 | 2º da lista | 37 |
| A2 | 3º da lista | 38 |
| A3 | 4º da lista | 39 |
| A4 | 5º da lista | 40 |

No GMRockKit mostrado na imagem, a ordem é Kick, Stick, Snare, Hand Clap e
Snare Rimshot. A porta A5 não é usada nesta configuração.

## Sensibilidade

O sketch dispara a partir da leitura analógica 20, captura o pico durante 4 ms
e converte a intensidade para velocidade MIDI de 40 a 127. Para ajustar à
montagem, altere `LIMIAR_TOQUE` (menor = mais sensível) e
`LEITURA_VELOCIDADE_MAXIMA` (menor = maior volume com batidas leves). O pad
só fica pronto para outra batida após a leitura cair abaixo de
`LIMIAR_LIBERACAO` por `TEMPO_LIBERACAO` milissegundos. Cada batida envia
apenas `note-on`, para o Hydrogen tocar a amostra inteira.
