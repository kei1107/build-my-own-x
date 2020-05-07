import * as fs from 'fs';
import * as bencode from 'bencode';
import * as crypto from 'crypto';
import * as bignum from 'bignum';

const open = (filepath: string): any => {
    console.log('open : ' + filepath);
    return bencode.decode(fs.readFileSync(filepath));
};
const size = (torrent: any): Buffer => {
    const size:number = torrent.info.files ? 
        torrent.info.files.map((file:string) => file.length).reduce((a:number,b:number) => a+b):
        torrent.info.length;
    return bignum.toBuffer(size, {endian:'big',size:8});
};
const infoHash = (torrent: any): Buffer => {
    const info: Buffer = bencode.encode(torrent.info);
    return crypto.createHash('sha1').update(info).digest();
};

export { open, size, infoHash };