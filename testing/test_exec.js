
import net from 'net';

const SOCKET_PATH = '/tmp/executor.sock';

const client = net.createConnection(SOCKET_PATH, () => {
    console.log('Connected to the C++ Executor!');


    for (let i = 0; i < 100; i++) {

        const message = {
            player_id: i,
            user_code: "print('hello world from player " + i + "')\nprnt('second part worked too!')",
            test_code: ""
        };

        const payload = JSON.stringify(message) + '\n';

        client.write(payload, () => {
        });
    }
});

client.on('end', () => {
    console.log('Disconnected from server');
});

client.on('error', (err) => {
    console.error('Socket error:', err.message);
});


let buffer = '';
client.on('data', (chunk) => {
    buffer += chunk.toString();
    let lines = buffer.split('\n');
    buffer = lines.pop(); // leftover

    for (let line of lines) {
        if (!line) continue;
        console.log('Received:', line);
    }
});
