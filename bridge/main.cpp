#include <jack/jack.h>
#include <jack/midiport.h>
#include <jack/ringbuffer.h>

#include <atomic>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <thread>
#include <unistd.h>
#include <termios.h>

struct MidiEvent {
    unsigned char data[3];
};

static jack_client_t* client = nullptr;
static jack_port_t* midiOut = nullptr;
static jack_ringbuffer_t* queue = nullptr;
static std::atomic<bool> running{true};

static void stop(int) {
    running = false;
}

static int process(jack_nframes_t frames, void*) {
    void* buffer = jack_port_get_buffer(midiOut, frames);
    jack_midi_clear_buffer(buffer);

    MidiEvent event;

    while (jack_ringbuffer_read_space(queue) >= sizeof(event)) {
        jack_ringbuffer_read(
            queue,
            reinterpret_cast<char*>(&event),
            sizeof(event)
        );

        jack_midi_event_write(
            buffer, 0, event.data, sizeof(event.data)
        );
    }

    return 0;
}

static int openSerial(const char* device) {
    int fd = open(device, O_RDONLY | O_NOCTTY | O_NONBLOCK);

    if (fd < 0) {
        perror(device);
        return -1;
    }

    termios tty{};

    if (tcgetattr(fd, &tty) != 0) {
        perror("tcgetattr");
        close(fd);
        return -1;
    }

    cfmakeraw(&tty);
    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);

    tty.c_cflag |= CLOCAL | CREAD;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        close(fd);
        return -1;
    }

    tcflush(fd, TCIFLUSH);
    return fd;
}

int main(int argc, char** argv) {
    const char* device = argc > 1 ? argv[1] : "/dev/ttyUSB0";

    signal(SIGINT, stop);
    signal(SIGTERM, stop);

    int fd = openSerial(device);

    if (fd < 0) {
        return 1;
    }

    queue = jack_ringbuffer_create(sizeof(MidiEvent) * 1024);

    if (!queue) {
        fprintf(stderr, "Erro ao criar fila MIDI\n");
        close(fd);
        return 1;
    }

    jack_status_t status;

    client = jack_client_open(
        "Arduino-Bateria", JackNullOption, &status
    );

    if (!client) {
        fprintf(stderr, "Erro ao conectar ao JACK\n");
        jack_ringbuffer_free(queue);
        close(fd);
        return 1;
    }

    midiOut = jack_port_register(
        client,
        "midi_out",
        JACK_DEFAULT_MIDI_TYPE,
        JackPortIsOutput,
        0
    );

    if (!midiOut) {
        fprintf(stderr, "Erro ao criar porta MIDI\n");
        jack_client_close(client);
        jack_ringbuffer_free(queue);
        close(fd);
        return 1;
    }

    jack_set_process_callback(client, process, nullptr);

    if (jack_activate(client) != 0) {
        fprintf(stderr, "Erro ao ativar cliente JACK\n");
        jack_client_close(client);
        jack_ringbuffer_free(queue);
        close(fd);
        return 1;
    }

    printf("Arduino MIDI: %s\n", device);
    printf("Saida JACK: Arduino-Bateria:midi_out\n");

    unsigned char packet[3];
    int position = 0;

    while (running) {
        unsigned char byte;
        ssize_t count = read(fd, &byte, 1);

        if (count == 1) {
            if (byte & 0x80) {
                if (byte == 0x90 || byte == 0x80) {
                    packet[0] = byte;
                    position = 1;
                } else {
                    position = 0;
                }
                continue;
            }

            if (position == 0) {
                continue;
            }

            packet[position++] = byte;

            if (position == 3) {
                MidiEvent event{};
                memcpy(event.data, packet, 3);

                if (jack_ringbuffer_write_space(queue) >= sizeof(event)) {
                    jack_ringbuffer_write(
                        queue,
                        reinterpret_cast<const char*>(&event),
                        sizeof(event)
                    );
                }

                position = 0;
            }
        } else if (count < 0 &&
                   errno != EAGAIN &&
                   errno != EWOULDBLOCK &&
                   errno != EINTR) {
            perror("serial read");
            break;
        } else {
            usleep(1000);
        }
    }

    jack_deactivate(client);
    jack_client_close(client);
    jack_ringbuffer_free(queue);
    close(fd);

    return 0;
}
