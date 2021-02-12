import * as fs from 'fs';
import * as bencode from 'bencode';
import * as crypto from 'crypto';
import * as bignum from 'bignum';

const BLOCK_LEN: number = Math.pow(2, 14);

const open = (filepath: string): any => {
    console.log('open : ' + filepath);
    return bencode.decode(fs.readFileSync(filepath));
};
const size = (torrent: any): Buffer => {
    const size: number = torrent.info.files ?
        torrent.info.files.map((file: string) => file.length).reduce((a: number, b: number) => a + b) :
        torrent.info.length;
    return bignum.toBuffer(size, { endian: 'big', size: 8 });
};
const infoHash = (torrent: any): Buffer => {
    const info: Buffer = bencode.encode(torrent.info);
    return crypto.createHash('sha1').update(info).digest();
};

const pieceLen = (torrent: any, pieceIndex: number): number => {
    const totalLength: number = bignum.fromBuffer(size(torrent)).toNumber();
    const pieceLength: number = torrent.info['piece length'];

    const lastPieceLength: number = totalLength % pieceLength;
    const lastPieceIndex: number = Math.floor(totalLength / pieceLength);

    return lastPieceIndex === pieceIndex ? lastPieceLength : pieceLength;
};

const blocksPerPiece = (torrent: any, pieceIndex: number): number => {
    const pieceLength: number = pieceLen(torrent, pieceIndex);
    return Math.ceil(pieceLength / BLOCK_LEN);
};

const blockLen = (torrent: any, pieceIndex: number, blockIndex: number) => {
    const pieceLength: number = pieceLen(torrent, pieceIndex);

    const lastPieceLength: number = pieceLength % BLOCK_LEN;
    const lastPieceIndex: number = Math.floor(pieceLength / BLOCK_LEN);

    return blockIndex === lastPieceIndex ? lastPieceLength : BLOCK_LEN;
};

export { BLOCK_LEN, open, size, infoHash, pieceLen, blocksPerPiece, blockLen };