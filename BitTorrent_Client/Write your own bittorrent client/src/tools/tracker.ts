import * as dgram from 'dgram';
import { parse as urlParse, UrlWithStringQuery } from 'url';
import * as crypto from 'crypto';
import * as util from './util';
import * as torrentParser from './torrent-parser';

type PeersType = {
    ip: string;
    port: number;
}[];
type BEP_ConnResp = {
    action: number,
    transactionId: number,
    connectionId: Buffer
};
type BEP_AnnounceResp = {
    action: number,
    transactionId: number,
    interval: number,
    leechers: number,
    seeders: number,
    peers: {
        ip: string,
        port: number,
    }[]
};

const getPeers = (torrent: any, callback: { (argv: PeersType): void }): void => {
    const socket: dgram.Socket = dgram.createSocket('udp4');
    const url: string = torrent.announce.toString('utf8');

    // 1. send connect request
    console.log('send connect request :[' + url + ']');
    udpSend(socket, buildConnReq(), url, () => { });

    console.log('start');
    socket.on('message', response => {
        const response_str: string = respType(response);
        console.log('response : ' + response_str);
        if (response_str === 'connect') {
            // 2. receive and parse connect response
            const connResp: BEP_ConnResp = parseConnResp(response);
            // 3. send announce request
            const announceReq: Buffer = buildAnnounceReq(connResp.connectionId, torrent);
            udpSend(socket, announceReq, url);
        } else if (response_str === 'announce') {
            // 4. parse announce response
            const announceResp: BEP_AnnounceResp = parseAnnounceResp(response);
            // 5. pass peers to callback
            callback(announceResp.peers);
        }
    });
};

function udpSend(socket: dgram.Socket, message: Buffer, rawUrl: string, callback = () => { }): void {
    const url: UrlWithStringQuery = urlParse(rawUrl);
    if (url.port !== null && url.hostname !== null) {
        socket.send(message, 0, message.length, parseInt(url.port), url.hostname, callback);
    }
};
function respType(resp: Buffer): string {
    const action: number = resp.readUInt32BE(0);
    if (action === 0) return 'connect';
    if (action === 1) return 'announce';
    return 'undefined'
};
function buildConnReq(): Buffer {
    const buf: Buffer = Buffer.alloc(16);

    // connection id
    buf.writeUInt32BE(0x417, 0);
    buf.writeUInt32BE(0x27101980, 4);
    // action
    buf.writeUInt32BE(0, 8);
    // transaction id
    crypto.randomBytes(4).copy(buf, 12);

    return buf;
};
function parseConnResp(resp: Buffer): BEP_ConnResp {
    return {
        action: resp.readUInt32BE(0),
        transactionId: resp.readUInt32BE(4),
        connectionId: resp.slice(8)
    }
};
function buildAnnounceReq(connId: Buffer, torrent: any, port = 6881): Buffer {
    const buf: Buffer = Buffer.allocUnsafe(98);

    // connection id
    connId.copy(buf, 0);
    // action
    buf.writeUInt32BE(1, 8);
    // transaction id
    crypto.randomBytes(4).copy(buf, 12);
    // info hash
    torrentParser.infoHash(torrent).copy(buf, 16);
    // peerId
    util.getId().copy(buf, 36);
    // downloaded
    Buffer.alloc(8).copy(buf, 56);
    // left
    torrentParser.size(torrent).copy(buf, 64);
    // uploaded
    Buffer.alloc(8).copy(buf, 72);
    // event
    buf.writeUInt32BE(0, 80);
    // ip address
    buf.writeUInt32BE(0, 84);
    // key
    crypto.randomBytes(4).copy(buf, 88);
    // num want
    buf.writeInt32BE(-1, 92);
    // port
    buf.writeUInt16BE(port, 96);
    return buf;
};
function parseAnnounceResp(resp: Buffer): BEP_AnnounceResp {
    function group(iterable: Buffer, groupSize: number) {
        let groups: Array<Buffer> = [];
        for (let i = 0; i < iterable.length; i += groupSize) {
            groups.push(iterable.slice(i, i + groupSize));
        }
        return groups;
    };
    return {
        action: resp.readUInt32BE(0),
        transactionId: resp.readUInt32BE(4),
        interval: resp.readUInt32BE(8),
        leechers: resp.readUInt32BE(12),
        seeders: resp.readUInt32BE(16),
        peers: group(resp.slice(20), 6).map(address => {
            return {
                ip: address.slice(0, 4).join('.'),
                port: address.readUInt16BE(4)
            }
        })
    }
};

export { getPeers };