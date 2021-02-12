import * as tp from './torrent-parser';
import * as message from './message';
import { QueueType } from './Queue'

export default class {
    _requested: boolean[][];
    _received: boolean[][];
    constructor(torrent: any) {
        function buildPiecesArray(): boolean[][] {
            const nPieces: number = torrent.info.pieces.length / 20;
            const arr: boolean[] = new Array(nPieces).fill(false);
            return arr.map((_, i) => new Array<boolean>(tp.blocksPerPiece(torrent, i)).fill(false));
        }
        this._requested = buildPiecesArray();
        this._received = buildPiecesArray();
    };

    addRequested(pieceBlock: QueueType): void {
        const blockIndex: number = pieceBlock.begin / tp.BLOCK_LEN;
        this._requested[pieceBlock.index][blockIndex] = true;
    };
    addReceived(pieceBlock: message.PayloadInParseType) {
        const blockIndex: number = pieceBlock.begin / tp.BLOCK_LEN;
        this._received[pieceBlock.index][blockIndex] = true;
    };

    needed(pieceBlock: QueueType): boolean {
        if (this._requested.every(blocks => blocks.every(i => i))) {
            this._requested = this._received.map(blocks => blocks.slice());
        }
        const blockIndex = pieceBlock.begin / tp.BLOCK_LEN;
        return !this._requested[pieceBlock.index][blockIndex];
    };
    isDone(): boolean {
        return this._received.every(blocks => blocks.every(i => i));
    };

    printPercentDone() {
        const downloaded: number = this._received.reduce((totalBlocks, blocks) => {
            return blocks.filter(i => i).length + totalBlocks;
        }, 0);

        const total: number = this._received.reduce((totalBlocks, blocks) => {
            return blocks.length + totalBlocks;
        }, 0);

        const percent: number = Math.floor(downloaded / total * 100);
        process.stdout.write('progress: ' + percent + '% (' + downloaded + ' / ' + total + ')\r');
    };
};