#include "Board.h"
#include "Patterns/PatternMatcher.h"

Board::Board(int boardSize)
{
    InitialiseEmpty(boardSize);
}

Board::Board(Colour colourToMove, const std::vector<std::string>& s)
{
    InitialiseEmpty(s.size());

    // Ensure that it's square.
    assert(s.size() == s[0].size());

    int loc;
    char c;
    for (int i = 0; i < _boardSize; i++)
    {
        for (int j = 0; j < _boardSize; j++)
        {
            loc = (_boardSize-i-1)*_boardSize + j;
            c = s[i][j];
            Colour col = c == 'B' ? Black : c == 'W' ? White : None;
            if (col != None)
                MakeMove({ col, loc });
        }
    }

    _colourToMove = colourToMove;
}

Board::Board(Colour colourToMove, int boardSize, const MoveHistory& history)
{
    assert(boardSize > 0 && boardSize < MaxBoardSize);

    InitialiseEmpty(boardSize);

    auto moves = history.Moves();
    for (Move move : moves)
        MakeMove(move);

    _colourToMove = colourToMove;
}

Board::~Board()
{
    if (_points != nullptr)
    {
        for (int i = 0; i < _boardArea; i++)
        {
            Point pt = _points[i];
            delete pt.Orthogonals;
            delete pt.Diagonals;
        }

        delete[] _points;
        _points = nullptr;
    }

    if (_blackStones != nullptr)
    {
        delete _blackStones;
        _blackStones = nullptr;
    }

    if (_whiteStones != nullptr)
    {
        delete _whiteStones;
        _whiteStones = nullptr;
    }

    auto it = _chains.begin();
    while (it != _chains.end())
    {
        StoneChain* c = *it;
        if (c != nullptr)
        {
            delete c->Stones;
            delete c->Neighbours;
            delete c;
            c = nullptr;
        }

        ++it;
    }
}

// Clone fields from other.
void Board::CloneFrom(const Board& other)
{
    assert(_boardSize == other._boardSize);

    _colourToMove = other._colourToMove;
    _turnNumber = other._turnNumber;
    _hashes = other._hashes;
    memcpy(_passes, other._passes, 2*sizeof(bool));

    // Copy the stones for each player.
    _empty->Copy(*other._empty);
    _blackStones->Copy(*other._blackStones);
    _whiteStones->Copy(*other._whiteStones);

    // Initialise the StoneChains for each point.
    for (int i = 0; i < _boardArea; i++)
    {
        Point& pt = _points[i];
        pt.Chain = new StoneChain*;
        *pt.Chain = nullptr;
    }

    // Copy the StoneChains and ensure that they are referenced by the right points.
    auto it = other._chains.begin();
    while (it != other._chains.end())
    {
        StoneChain* oc = *it;
        StoneChain c = { oc->Col, new BitSet(*oc->Stones), new BitSet(*oc->Neighbours), oc->Hash };

        // Find the points with this StoneChain.
        for (int i = 0; i < _boardArea; i++)
        {
            const Point& opt = other._points[i];
            if (*opt.Chain == oc)
            {
                *_points[i].Chain = &c;
            }
        }

        _chains.push_back(&c);
        ++it;
    }
}

// Check for potential eyes.
// All orthogonals must be the same colour and there is also a constraint on diagonals.
bool Board::IsEye(Colour col, int loc) const
{
    const Point& pt = _points[loc];
    bool isEye = pt.Col() == None;

    if (isEye)
    {
        const BitSet* const friendly = col == Black ? _blackStones : _whiteStones;
        const BitSet* const enemy = col == Black ? _whiteStones : _blackStones;
        int orth = pt.Orthogonals->CountAnd(*friendly);
        int enemyDiag = pt.Diagonals->CountAnd(*enemy);

        size_t n = pt.Neighbours.size();
        isEye = n == 2 ? orth == 2 && enemyDiag == 0:
                n == 3 ? orth == 3 && enemyDiag == 0:
                n == 4 ? orth == 4 && enemyDiag <= 1:
                false;
    }

    return isEye;
}

// Check the legality of the specified move in this position.
MoveInfo Board::CheckMove(int loc, bool duringPlayout) const
{
    return CheckMove(_colourToMove, loc, duringPlayout);
}

// Check the legality of the specified move in this position.
MoveInfo Board::CheckMove(Colour col, int loc, bool duringPlayout) const
{
    // Passing is always legal.
    if (loc == PassCoord)
        return Legal;

    // Check occupancy.
    const Point& pt = _points[loc];
    MoveInfo res = pt.Col() == None ? Legal : Occupied;
    if (res == Legal)
    {
        // Check for suicide and ko.
        int liberties = 0;
        int captureLoc;
        size_t capturesWithRepetition = 0; // Note: captured neighbours could be in the same group!
        size_t friendlyOrthogonals = 0;
        bool friendInAtari = false;
        bool isAtari = false;
        for (const Point* const n : pt.Neighbours)
        {
            if (n->Col() == None)
            {
                ++liberties;
            }
            else if (n->Col() == col)
            {
                int nlibs = CountChainLiberties(*n->Chain);
                liberties += nlibs-1;
                friendlyOrthogonals += nlibs > 1 ? 1 : 0;
                friendInAtari = friendInAtari || nlibs == 1;
            }
            else
            {
                int nlibs = CountChainLiberties(*n->Chain);
                if (nlibs == 1)
                {
                    ++liberties;
                    captureLoc = n->Coord;
                    capturesWithRepetition += CountChainSize(*n->Chain);
                }
                else if (nlibs == 2)
                {
                    isAtari = true;
                }
            }
        }

        bool suicide = liberties == 0;
        bool repetition = capturesWithRepetition == 1 && IsKoRepetition(col, loc, captureLoc);
        res = suicide ? Suicide : repetition ? Ko : Legal;

        if (res & Legal)
        {
            // Do some extra work to further classify the move.
            if (isAtari) res |= Atari;
            if (liberties == 1) res |= SelfAtari;
            if (capturesWithRepetition > 0) res |= Capture;
            if (friendInAtari && liberties > 1) res |= Save;
            if (IsEye(col, loc)) res |= FillsEye;
        }
    }

    return res;
}

// Get all moves available for the current colour.
std::vector<Move> Board::GetMoves(bool duringPlayout) const
{
    std::vector<Move> moves;
    if (!GameOver())
    {
        PatternMatcher matcher;
        for (int i = 0; i < _boardArea; i++)
        {
            MoveInfo info = CheckMove(i, duringPlayout);
            if (info & Legal)
            {
                if (!duringPlayout)
                {
                    if (matcher.HasMatch(*this, 3, i))
                    {
                        info |= Pat3Match;
                    }

                    if (matcher.HasMatch(*this, 5, i))
                    {
                        info |= Pat5Match;
                    }
                }

                if (!(info & FillsEye))
                {
                    moves.push_back({this->_colourToMove, i, info});
                }
            }
        }

        // Passing is always legal but don't consider it during a playout unless there
        // are no other moves available.
        if (!duringPlayout || moves.size() == 0)
        {
            moves.push_back({this->_colourToMove, PassCoord, Legal});
        }
    }

    return moves;
}

// Update the board state with the specified move.
void Board::MakeMove(const Move& move)
{
    assert(!GameOver());
    assert(CheckMove(move.Col, move.Coord) & Legal);

    auto z = Zobrist::Instance();
    uint64_t nextHash = _hashes[_turnNumber-1];

    if (move.Coord != PassCoord)
    {
        Point& pt = _points[move.Coord];

        // Create a new chain.
        uint64_t ch = z->Key(move.Col, move.Coord);

        StoneChain* next = new StoneChain;
        next->Col = move.Col;
        next->Stones = new BitSet(_boardArea);
        next->Neighbours = new BitSet(*pt.Orthogonals);
        next->Hash = ch;

        next->Stones->Set(move.Coord);

        BitSet* friendly = move.Col == Black ? _blackStones : _whiteStones;
        BitSet* enemy = move.Col == Black ? _whiteStones : _blackStones;

        // Update the cached BitSets.
        friendly->Set(move.Coord);
        _empty->UnSet(move.Coord);

        // First detect chains which will get captured and remove them.
        for (Point* const n : pt.Neighbours)
        {
            StoneChain* c = *n->Chain;
            if (c != nullptr)
            {
                
                if (c->Col != move.Col && CountChainLiberties(c) == 1)
                {
                    // Capture this enemy group.
                    nextHash ^= c->Hash;
                    enemy->UnSet(*c->Stones);
                    _empty->Set(*c->Stones);
                    RemoveChain(c);
                }
                else if (n->Col() == move.Col)
                {
                    // Friendly chain - need to merge.
                    MergeChains(next, c);

                    // Remove this merged group.
                    _chains.remove(c);
                    delete c;

                    // Repoint at the new merged group.
                    c = next;
                }
            }
        }

        _chains.push_back(next);
        pt.Chain = &next;
    }

    // Update the colour to move.
    if (move.Col == _colourToMove)
    {
        _colourToMove = _colourToMove == Black ? White : Black;
        nextHash ^= CurrentRules.Ko == Situational ? z->BlackTurn() : 0;
    }

    _hashes.push_back(nextHash);
    _passes[(int)move.Col-1] = move.Coord == PassCoord;
    ++_turnNumber;
}

// Compute the score of the current position.
// Area scoring is used and we assume that all empty points are fully surrounded by one
// colour.
// The score is determined from black's perspective (positive score indicates black win).
int Board::Score() const
{
    double score = -CurrentRules.Komi;
    for (int i = 0; i < _boardArea; i++)
    {
        const Point& pt = _points[i];
        if (pt.Col() == None)
        {
            // Look at a neighbour.
            Point const* const n = pt.Neighbours[0];
            score += n->Col() == Black ? 1 : -1;
        }
        else
        {
            score += pt.Col() == Black ? 1 : -1;
        }
    }

    return score > 0 ? 1 : score < 0 ? -1 : 0;
}

std::string Board::ToString() const
{
    std::string s;
    Colour col;
    for (int r = _boardSize-1; r >= 0; r--)
    {
        for (int c = 0; c < _boardSize; c++)
        {
            col = _points[r*_boardSize+c].Col();
            s += col == None ? '.' : col == Black ? 'B' : 'W';
        }

        s += '\n';
    }

    return s;
}

// Initialise an empty board of the specified size.
void Board::InitialiseEmpty(int boardSize)
{
    assert(boardSize > 0 && boardSize < MaxBoardSize);

    _colourToMove = Black;
    _boardSize = boardSize;
    _boardArea = _boardSize*_boardSize;

    _empty = new BitSet(_boardArea);
    _empty->Invert();

    _blackStones = new BitSet(_boardArea);
    _whiteStones = new BitSet(_boardArea);
    _points = new Point[_boardArea];
    for (int i = 0; i < _boardArea; i++)
        _points[i] = { None, {}, nullptr, nullptr, nullptr };

    InitialiseNeighbours();
    _hashes.push_back(CurrentRules.Ko == Situational ? Zobrist::Instance()->BlackTurn() : 0);
}

// Initialise the neighbours for each point.
void Board::InitialiseNeighbours()
{
    for (int i = 0; i < _boardArea; i++)
    {
        _points[i].Coord = i;

        _points[i].Neighbours.clear();
        _points[i].Orthogonals = new BitSet(_boardArea);
        _points[i].Diagonals = new BitSet(_boardArea);

        // Setup the orthogonals.
        // Also fill in the neighbours vector at this point.
        int orth[4] = { i-_boardSize, i+_boardSize, i-1, i+1 };
        for (int i = 0; i < 4; i++)
        {
            _points[i].Neighbours.push_back(&_points[orth[i]]);
            _points[i].Orthogonals->Set(orth[i]);
        }

        // Setup the diagonals.
        int diag[4] = { i-1-_boardSize, i+1-_boardSize, i-1+_boardSize, i+1+_boardSize };
        for (int i = 0; i < 4; i++)
        {
            _points[i].Diagonals->Set(diag[i]);
        }
    }
}

// Check whether the specified move and capture would result in a board repetition.
bool Board::IsKoRepetition(Colour col, int loc, int captureLoc) const
{
    Colour enemyCol = col == Black ? White : Black;
    auto z = Zobrist::Instance();
    uint64_t nextHash = _hashes[_turnNumber-1]
        ^ z->Key(col, loc)
        ^ z->Key(enemyCol, captureLoc)
        ^ (CurrentRules.Ko == Situational ? z->BlackTurn() : 0);

    // Has this hash occurred previously?
    bool repeat = false;
    for (int i = _turnNumber-1; i >= 0 && !repeat; i--)
        repeat = _hashes[i] == nextHash;

    return repeat;
}

void Board::RemoveChain(StoneChain* chain)
{
    _chains.remove(chain);
    delete chain;
    chain = nullptr;
}

int Board::CountChainLiberties(StoneChain* chain) const
{
    return chain->Neighbours->CountAnd(*_empty);
}

int Board::CountChainSize(StoneChain* chain) const
{
    return chain->Stones->Count();
}

void Board::MergeChains(StoneChain* base, StoneChain* other) const
{
    base->Stones->Set(*other->Stones);
    base->Neighbours->Set(*other->Neighbours);
    base->Neighbours->UnSet(*base->Stones);
    base->Hash ^= other->Hash;
}












