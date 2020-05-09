import * as torrentParser from './torrent-parser';
import * as util from './util';
import { QueueType } from './Queue'

// ==================================================================== //
// https://wiki.theory.org/index.php/BitTorrentSpecification#Handshake  //
const buildHandshake = (torrent: any): Buffer => {
    const buf: Buffer = Buffer.alloc(68);
    // pstrlen
    buf.writeUInt8(19, 0);
    // pstr
    buf.write('BitTorrent protocol', 1);
    // received
    buf.writeUInt32BE(0, 20);
    buf.writeUInt32BE(0, 24);
    // info hash
    torrentParser.infoHash(torrent).copy(buf, 28);
    // peer id
    util.getId().copy(buf, 48);
    return buf;
};
// ==================================================================== //
// https://wiki.theory.org/index.php/BitTorrentSpecification#Messages   //

const buildKeepAlive = (): Buffer => Buffer.alloc(4);

const buildChoke = (): Buffer => {
    const buf: Buffer = Buffer.alloc(5);
    // length
    buf.writeUInt32BE(1, 0);
    // id
    buf.writeUInt8(0, 4);
    return buf;
};

const buildUnchoke = (): Buffer => {
    const buf: Buffer = Buffer.alloc(5);
    // length
    buf.writeUInt32BE(1, 0);
    // id
    buf.writeUInt8(1, 4);
    return buf;
};

const buildInterested = (): Buffer => {
    const buf: Buffer = Buffer.alloc(5);
    // length
    buf.writeUInt32BE(1, 0);
    // id
    buf.writeUInt8(2, 4);
    return buf;
};

const buildUninterested = (): Buffer => {
    const buf: Buffer = Buffer.alloc(5);
    // length
    buf.writeUInt32BE(1, 0);
    // id
    buf.writeUInt8(3, 4);
    return buf;
};

const buildHave = (payload: number): Buffer => {
    const buf: Buffer = Buffer.alloc(9);
    // length
    buf.writeUInt32BE(5, 0);
    // id
    buf.writeUInt8(4, 4);
    // piece index
    buf.writeUInt32BE(payload, 5);
    return buf;
};

/* TODO : make bitfield type 
const buildBitfield = (payload: bitfield): Buffer => {
    const buf: Buffer = Buffer.alloc(14);
    // length
    buf.writeUInt32BE(payload.length + 1, 0);
    // id
    buf.writeUInt8(5, 4);
    // bitfield
    payload.copy(buf,5);
    return buf;
};
//*/

const buildRequest = (payload: QueueType): Buffer => {
    const buf = Buffer.alloc(17);
    // length
    buf.writeUInt32BE(13, 0);
    // id
    buf.writeUInt8(6, 4);
    // piece index
    buf.writeUInt32BE(payload.index, 5);
    // begin
    buf.writeUInt32BE(payload.begin, 9);
    // length
    buf.writeUInt32BE(payload.length, 13);
    return buf;
};

// paylod: Pieces
// const buildPiece = (payload: any): Buffer => {
//     const buf = Buffer.alloc(payload.block.length + 13);
//     // length
//     buf.writeUInt32BE(payload.block.length + 9, 0);
//     // id
//     buf.writeUInt8(7, 4);
//     // piece index
//     buf.writeUInt32BE(payload.index, 5);
//     // begin
//     buf.writeUInt32BE(payload.begin, 9);
//     // block
//     payload.block.copy(buf, 13);
//     return buf;
// };

// payload: Queue
const buildCancel = (payload: QueueType): Buffer => {
    const buf = Buffer.alloc(17);
    // length
    buf.writeUInt32BE(13, 0);
    // id
    buf.writeUInt8(8, 4);
    // piece index
    buf.writeUInt32BE(payload.index, 5);
    // begin
    buf.writeUInt32BE(payload.begin, 9);
    // length
    buf.writeUInt32BE(payload.length, 13);
    return buf;
};

const buildPort = (payload: number): Buffer => {
    const buf = Buffer.alloc(7);
    // length
    buf.writeUInt32BE(3, 0);
    // id
    buf.writeUInt8(9, 4);
    // listen-port
    buf.writeUInt16BE(payload, 5);
    return buf;
};
// ==================================================================== //
type ParseType = {
    size: number,
    id: number | null,
    payload: PayloadInParseType | Buffer | null
};
type PayloadInParseType = {
    index: number,
    begin: number,
    block: Buffer | null,
    length: Buffer | null
};

const parse = (msg: Buffer): ParseType => {
    const id: number | null = msg.length > 4 ? msg.readInt8(4) : null;
    const payload: Buffer | null = msg.length > 5 ? msg.slice(5) : null;
    let ret_payload: PayloadInParseType | Buffer | null = payload;
    if (id === 6 || id === 7 || id === 8) {
        if (payload != null) {
            const rest: Buffer = payload.slice(8);
            ret_payload = {
                index: payload.readInt32BE(0),
                begin: payload.readInt32BE(0),
                block: null,
                length: null
            };
            ret_payload[id === 7 ? 'block' : 'length'] = rest;
        }
    }
    return {
        size: msg.readInt32BE(0),
        id: id,
        payload: ret_payload
    }
};
// ==================================================================== //

export {
    buildHandshake,
    buildKeepAlive,
    buildChoke,
    buildUnchoke,
    buildInterested,
    buildUninterested,
    buildHave,
    // buildBitfield,
    buildRequest,
    // buildPiece,
    buildCancel,
    buildPort,
    parse,
    ParseType,
    PayloadInParseType
};