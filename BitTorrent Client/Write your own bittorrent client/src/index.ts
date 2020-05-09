import * as torrentParser from './tools/torrent-parser';
import * as download from './tools/download';

const torrent: any = torrentParser.open(process.argv[2]);
download.start(torrent, torrent.info.name);
