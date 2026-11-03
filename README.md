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

| Pad | Instrumento | Nota MIDI |
|-----|-------------|-----------|
| 1 | Bumbo | 36 |
| 2 | Caixa | 38 |
| 3 | Chimbal fechado | 42 |
| 4 | Chimbal aberto | 46 |
| 5 | Tom | 45 |
| 6 | Crash | 49 |
