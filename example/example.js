
import net from 'net';






const executor = net.createConnection('/tmp/executor.sock');

executor.on('end', () => {
    console.log('Disconnected from executor');
});

executor.on('error', (err) => {
    console.error('Socket error:', err.message);
});


const setExecutorOnMessage = (executor, onMessage) => { 
    let buffer = '';

    executor.on('data', (chunk) => {
        buffer += chunk.toString();

        while (true) {
            const idx = buffer.indexOf('\n');
            if (idx === -1) break;

            const line = buffer.slice(0, idx);

            const message = JSON.parse(line);

            onMessage(message)

            buffer = buffer.slice(idx + 1);
            if (!line) continue;
        }
    });


    executor.on('end', () => {
        console.log('Disconnected from executor');
    });

    executor.on('error', (err) => {
        console.error('Socket error:', err.message);
    });
}

const requestCodeExecution = (executor, message) => {
    const payload = JSON.stringify(message) + '\n';
    executor.write(payload);
}


setExecutorOnMessage(executor, (message) => console.log(message));


requestCodeExecution(executor, {
    game_id: 1,
    player_id: 2,
    user_code: "print('hello world')\n",
    test_code: "",
    inputs_code: ["", ""]
})












