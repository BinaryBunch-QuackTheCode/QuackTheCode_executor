
const net = require('net');
const { setTimeout } = require('timers/promises'); 

const SOCKET_PATH = '/tmp/executor.sock';

const client = net.createConnection(SOCKET_PATH, async () => {
    console.log('Connected to the C++ Executor!');

    const message = {
        script: "print('Hello from Node.js!')"
    };

    const payload = JSON.stringify(message) + '\n';

    const middle = payload.length / 2; 

    const firstHalf = payload.slice(0, middle);  
    const secondHalf = payload.slice(middle);

    client.write(firstHalf, () => {
        console.log('First half sent');
    });

    await setTimeout(1000);

    client.write(secondHalf, () => {
        console.log('Second half sent.');
    });

    console.log("Sent " + payload.length + " bytes");
});

client.on('error', (err) => {
    console.error('Socket error:', err.message);
});
