
import net from 'net';

const TOTAL_MESSAGES = 100;
let responses = 0;
let startTime = null;

let buffer = '';

const client = net.createConnection('/tmp/executor.sock', () => {
    console.log('Connected to the Executor');

    startTime = process.hrtime.bigint();

    for (let i = 0; i < TOTAL_MESSAGES; i++) {
        const message = {
            player_id: i,
            user_code: "print('hello world')\n",
            test_code: ""
        };

        const payload = JSON.stringify(message) + '\n';
        client.write(payload);
    }
});

client.on('data', (chunk) => {
    buffer += chunk.toString();

    while (true) {
        const idx = buffer.indexOf('\n');
        if (idx === -1) break;

        const line = buffer.slice(0, idx);
        console.log(line);
        buffer = buffer.slice(idx + 1);

        if (!line) continue;

        responses++;

        if (responses === TOTAL_MESSAGES) {
            const endTime = process.hrtime.bigint();
            const elapsedMs = Number(endTime - startTime) / 1e6;

            console.log(`All ${TOTAL_MESSAGES} responses received in ${elapsedMs.toFixed(2)} ms`);
            client.end();
        }
    }
});

client.on('end', () => {
    console.log('Disconnected from server');
});

client.on('error', (err) => {
    console.error('Socket error:', err.message);
});
