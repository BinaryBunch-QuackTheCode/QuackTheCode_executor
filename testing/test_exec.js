
const net = require('net');

const SOCKET_PATH = '/tmp/executor.sock';

const client = net.createConnection(SOCKET_PATH, () => {
    console.log('Connected to the C++ Executor!');

    const message = {
        player_id: 1,
        user_code: "print('hello world')",
        test_code: ""
    };

    const payload = JSON.stringify(message) + '\n';

    client.write(payload, () => {
        console.log('Payload sent');
    });

});

client.on('error', (err) => {
    console.error('Socket error:', err.message);
});
 
client.on('data', (data) => {
    console.log(data);
})

