import * as fs from 'fs';
import * as bencode from 'bencode';
import * as tracker from './tracker';
import * as torrentParser from './torrent-parser';

const torrent: any = torrentParser.open('puppy2.torrent');
tracker.getPeers(torrent, peers=>{
    console.log('list of peers: ',peers);
});
