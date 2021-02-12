import * as fs from 'fs'
import * as net from 'net';
import * as tracker from './tracker';
import * as message from './message';
import Pieces from './Pieces';
import Queue, { QueueType } from './Queue'

const start = (torrent: any, path: Buffer): void => {
    tracker.getPeers(torrent, peers => {
        console.log('list of peers: ', peers);
        console.log("Download to ", path.toString());
        const pieces: Pieces = new Pieces(torrent);
        const file: number = fs.openSync(path, 'w');
        peers.forEach(peer => download(peer, torrent, pieces, file));
    });
};
function download(peer: any, torrent: any, pieces: Pieces, file: number) {
    const socket: net.Socket = new net.Socket();
    socket.on('error', console.log);
    socket.connect(peer.port, peer.ip, () => {
        socket.write(message.buildHandshake(torrent));
    });

    const queue: Queue = new Queue(torrent);
    onWholeMsg(socket, msg => msgHandler(msg, socket, pieces, queue, torrent, file));
};

function onWholeMsg(socket: net.Socket, callback: { (argv: Buffer): void }) {
    let savedBuf: Buffer = Buffer.alloc(0);
    let handshake: boolean = true;

    socket.on('data', recvBuf => {
        // msgLen calculates the length of a whole message
        const msgLen = () => handshake ? savedBuf.readUInt8(0) + 49 : savedBuf.readInt32BE(0) + 4;
        savedBuf = Buffer.concat([savedBuf, recvBuf]);

        while (savedBuf.length >= 4 && savedBuf.length >= msgLen()) {
            callback(savedBuf.slice(0, msgLen()));
            savedBuf = savedBuf.slice(msgLen());
            handshake = false;
        }
    });
};

function msgHandler(msg: Buffer, socket: net.Socket, pieces: Pieces, queue: Queue, torrent: any, file: number) {
    if (isHandshake(msg)) {
        socket.write(message.buildInterested());
    } else {
        const m: message.ParseType = message.parse(msg);

        if (m.id === 0) chokeHandler(socket);
        if (m.id === 1) unchokeHandler(socket, pieces, queue);
        if (m.id === 4 && m.payload instanceof Buffer) haveHandler(socket, pieces, queue, m.payload);
        if (m.id === 5 && m.payload instanceof Buffer) bitfieldHandler(socket, pieces, queue, m.payload);
        if (m.id === 7 && m.payload !== null && !(m.payload instanceof Buffer)) pieceHandler(socket, pieces, queue, torrent, file, m.payload);
    }
};
function isHandshake(msg: Buffer) {
    return msg.length === msg.readUInt8(0) + 49 &&
        msg.toString('utf8', 1, 20) === 'BitTorrent protocol';
};

function chokeHandler(socket: net.Socket) {
    console.log("Receive chocke! Shutdown.")
    socket.end();
};

function unchokeHandler(socket: net.Socket, pieces: Pieces, queue: Queue) {
    queue.choked = false;
    requestPiece(socket, pieces, queue);
};

function haveHandler(socket: net.Socket, pieces: Pieces, queue: Queue, payload: Buffer) {
    const pieceIndex: number = payload.readUInt32BE(0);
    const queueEmpty: boolean = queue.length() === 0;
    queue.queue(pieceIndex);
    if (queueEmpty) requestPiece(socket, pieces, queue);
};

function bitfieldHandler(socket: net.Socket, pieces: Pieces, queue: Queue, payload: Buffer) {
    const queueEmpty: boolean = queue.length() === 0;
    payload.forEach((byte, i) => {
        for (let j = 0; j < 8; j++) {
            if (byte % 2) queue.queue(i * 8 + 7 - j);
            byte = Math.floor(byte / 2);
        }
    });
    if (queueEmpty) requestPiece(socket, pieces, queue);
};

function pieceHandler(socket: net.Socket, pieces: Pieces, queue: Queue, torrent: any, file: number, pieceResp: message.PayloadInParseType) {
    pieces.printPercentDone();
    pieces.addReceived(pieceResp);

    const offset: number = pieceResp.index * torrent.info['piece length'] + pieceResp.begin;

    if (pieceResp.block !== null) {
        fs.write(file, pieceResp.block, 0, pieceResp.block.length, offset, () => { });
    }
    if (pieces.isDone()) {
        console.log("DONE!");
        socket.end();
        try {
            fs.closeSync(file);
        } catch (e) {
            console.log("Error:", e);
        }
    } else {
        requestPiece(socket, pieces, queue);
    }
};

function requestPiece(socket: net.Socket, pieces: Pieces, queue: Queue) {
    if (queue.choked) return null;

    while (queue.length()) {
        const pieceBlock: QueueType | undefined = queue.deque();
        if (pieceBlock !== undefined) {
            if (pieces.needed(pieceBlock)) {
                socket.write(message.buildRequest(pieceBlock));
                pieces.addRequested(pieceBlock);
                break;
            }
        }
    }
};

export { start };
