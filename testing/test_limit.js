import net from 'net';

let buffer = '';

const client = net.createConnection('/tmp/executor.sock', () => {
    console.log('Connected to the Executor');

    const message = {
        player_id: 67,
        user_code: "x=5\nfor i in range(999999999):\n\tx+=1\nprint('done')\n",
        test_code: ""
    };

    client.write(JSON.stringify(message) + '\n');
});

client.on('data', (chunk) => {
    buffer += chunk.toString();

    while (true) {
        const idx = buffer.indexOf('\n');
        if (idx === -1) break;

        const line = buffer.slice(0, idx);
        buffer = buffer.slice(idx + 1);

        if (!line) continue;

        console.log(line);

    }
});

client.on('end', () => {
    console.log('Disconnected from server');
});

client.on('error', (err) => {
    console.error('Socket error:', err.message);
});

