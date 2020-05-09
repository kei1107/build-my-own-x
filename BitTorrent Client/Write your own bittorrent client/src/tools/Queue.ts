import * as tp from './torrent-parser';

type QueueType = {
    index: number,
    begin: number,
    length: number
};

export default class {
    _torrent: any;
    _queue: QueueType[];
    choked: boolean;

    constructor(torrent: any) {
        this._torrent = torrent;
        this._queue = [];
        this.choked = true;
    };

    queue(pieceIndex: number) {
        const nBlocks = tp.blocksPerPiece(this._torrent, pieceIndex);
        for (let i = 0; i < nBlocks; i++) {
            const pieceBlock = {
                index: pieceIndex,
                begin: i * tp.BLOCK_LEN,
                length: tp.blockLen(this._torrent, pieceIndex, i)
            };
            this._queue.push(pieceBlock);
        }
    };

    deque(): QueueType | undefined { return this._queue.shift(); }
    peek(): QueueType { return this._queue[0]; }
    length(): number { return this._queue.length; }
};

export { QueueType };