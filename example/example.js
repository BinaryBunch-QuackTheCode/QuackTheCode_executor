
import net from 'net';
import fs from 'node:fs';


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
        executor.end()
    });

}

const requestCodeExecution = (executor, message) => {
    const payload = JSON.stringify(message) + '\n';
    executor.write(payload);
}

setExecutorOnMessage(executor, (message) => console.log(message));

if (process.argv.length < 3) { 
    console.error("Usage: node example.js <python script to run>")
    process.exit(1);
}

fs.readFile(process.argv[2], 'utf8', (err, data) => {
    if (err) {
        console.error(err);
        return;
    }
    requestCodeExecution(executor, {
        game_id: 1, 
        player_id: 2, 
        user_code: data, 
        test_code: "", 
        inputs_code: [""]
    });

});








