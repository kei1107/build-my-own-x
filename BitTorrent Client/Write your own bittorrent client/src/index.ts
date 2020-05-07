import * as fs from 'fs';
import * as bencode from 'bencode';

const torrent:any = bencode.decode(fs.readFileSync('puppy.torrent'));
console.log(torrent.announce.toString('utf8'));
