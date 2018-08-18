#include "Board.h"
#include <iostream>

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

    for (StoneChain& sc : _chains)
    {
        if (sc.Stones != nullptr)
        {
            delete sc.Stones;
            sc.Stones = nullptr;
        }

        if (sc.Neighbours != nullptr)
        {
            delete sc.Neighbours;
            sc.Neighbours = nullptr;
        }
    }
}

// Clone fields from other.
void Board::CloneFrom(const Board& other)
{
    assert(_boardSize == other._boardSize);

    // Delete the existing chains 
    for (size_t i = 0; i < _chains.size(); i++)
    {
        const StoneChain& c = _chains[i];
        delete c.Stones;
        delete c.Neighbours;
    }

    _chains.clear();

    _colourToMove = other._colourToMove;
    _turnNumber = other._turnNumber;
    _lastMove = other._lastMove;
    _hashes = other._hashes;
    memcpy(_passes, other._passes, 2*sizeof(bool));

    // Copy the stones for each player.
    _empty->Copy(*other._empty);
    _blackStones->Copy(*other._blackStones);
    _whiteStones->Copy(*other._whiteStones);

    // Copy the stone chains.
    for (size_t i = 0; i < other._chains.size(); i++)
    {
        const StoneChain& oc = other._chains[i];
        BitSet* stones = new BitSet(*oc.Stones);
        BitSet* neighbours = new BitSet(*oc.Neighbours);

        StoneChain c = { oc.Col, oc.Liberties, stones, neighbours, oc.Hash, oc.Ignore };
        _chains.push_back(c);
    }

    // Map the points to their stone chains.
    for (int i = 0; i < _boardArea; i++)
    {
        const Point& op = other._points[i];
        _points[i].Col = op.Col;
        _points[i].ChainId = op.ChainId;
    }
}

// Check for potential eyes.
// All orthogonals must be the same colour and there is also a constraint on diagonals.
bool Board::IsEye(Colour col, int loc, int orth) const
{
    const Point& pt = _points[loc];
    bool isEye = pt.Col == None;

    if (isEye)
    {
        const BitSet* const enemy = col == Black ? _whiteStones : _blackStones;
        int enemyDiag = pt.Diagonals->CountAndSparse(*enemy);

        size_t n = pt.Neighbours.size();
        isEye = n == 2 ? orth == 2 && enemyDiag == 0:
                n == 3 ? orth == 3 && enemyDiag == 0:
                n == 4 ? orth == 4 && enemyDiag <= 1:
                false;
    }

    return isEye;
}

// Check the legality of the specified move in this position.
MoveInfo Board::CheckMove(int loc) const
{
    return CheckMove(_colourToMove, loc);
}

// Check the legality of the specified move in this position.
MoveInfo Board::CheckMove(Colour col, int loc) const
{
    // Passing is always legal.
    if (loc == PassCoord)
        return Legal;

    // Check occupancy.
    const Point& pt = _points[loc];
    MoveInfo res = pt.Col == None ? Legal : Occupied;
    if (res == Legal)
    {
        // Check for suicide and ko.
        int liberties = 0;
        int captureLoc;
        size_t capturesWithRepetition = 0; // Note: captured neighbours could be in the same group!
        size_t friendlyOrthogonals = 0;
        bool friendInAtari = false;
        bool isAtari = false;
        bool isLocal = false;
        for (const Point* const n : pt.Neighbours)
        {
            const StoneChain& c = _chains[n->ChainId];
            if (n->Col == None)
            {
                ++liberties;
            }
            else if (n->Col == col)
            {
                int nlibs = c.Liberties;
                liberties += nlibs-1;
                friendlyOrthogonals += nlibs > 1 ? 1 : 0;
                friendInAtari = friendInAtari || nlibs == 1;
            }
            else
            {
                int nlibs = c.Liberties;
                if (nlibs == 1)
                {
                    ++liberties;
                    captureLoc = n->Coord;
                    capturesWithRepetition += CountChainSize(n->ChainId);
                }
                else if (nlibs == 2)
                {
                    isAtari = true;
                }

                isLocal |= n->Coord == _lastMove.Coord;
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
            if (IsEye(col, loc, friendlyOrthogonals)) res |= FillsEye;
            if (isLocal) res |= Local;
        }
    }

    return res;
}

// Get all moves available for the current colour.
std::vector<Move> Board::GetMoves(bool duringPlayout) const
{
    std::vector<Move> moves;
    moves.reserve(_empty->Count());
    if (!GameOver())
    {
        for (int i = 0; i < _boardArea; i++)
        {
            MoveInfo info = CheckMove(i);
            if ((info & Legal) && !(info & FillsEye))
            {
                moves.push_back({_colourToMove, i, info});
            }
        }

        // Passing is always legal but don't consider it during a playout unless there
        // are no other moves available.
        if (!duringPlayout || moves.size() == 0)
        {
            moves.push_back({_colourToMove, PassCoord, Legal});
        }
    }

    return moves;
}

// Get n random (legal) moves.
std::vector<Move> Board::GetRandomLegalMoves(size_t n, RandomGenerator& gen) const
{
    std::vector<Move> moves;
    moves.reserve(n);
    if (!GameOver())
    {
        // Can easily generate candidate moves by looking at the empty locations.
        int numEmpty = _empty->Count();
        assert(numEmpty != 0);

        int maxAttempts = 5*n;
        int a = 0, loc;
        BitSelector bsl(*_empty);
        while (a++ < maxAttempts && moves.size() < n)
        {
            // Check a random empty point for validity.
            loc = bsl[gen.Next(numEmpty)];

            MoveInfo info = CheckMove(loc);
            bool legal = info & Legal;
            bool fillsEye = info & FillsEye;
            if (legal && !fillsEye)
            {
                moves.push_back({_colourToMove, loc, info});
            }
        }
    }

    // If no legal moves have been found then fall back on full moves search to ensure that the
    // game is played out fully.
    if (moves.empty())
    {
        moves = GetMoves(true);
    }

    return moves;
}

// Find a global move that is adjacent to an enemy group with the specified number of liberties.
Move Board::GetRandomMoveAttackingLiberties(int liberties, RandomGenerator& gen) const
{
    std::vector<Move> attackingMoves;
    for (const auto& chain : _chains)
    {
        if (!chain.Ignore && chain.Col != _colourToMove && chain.Liberties == liberties)
        {
            FindLegalLibertyMoves(chain, attackingMoves);
        }
    }

    return attackingMoves.empty() ? BadMove : attackingMoves[gen.Next(attackingMoves.size())];
}

// Find a global move that is adjacent to a friendly group with only one liberty.
Move Board::GetRandomMoveSaving(RandomGenerator& gen) const
{
    std::vector<Move> savingMoves;
    for (const auto& chain : _chains)
    {
        if (!chain.Ignore && chain.Col == _colourToMove && chain.Liberties == 1)
        {
            FindLegalLibertyMoves(chain, savingMoves);
        }
    }

    return savingMoves.empty() ? BadMove : savingMoves[gen.Next(savingMoves.size())];
}

// Find a move that is local to the previous move and is considered urgent.
// Note: Adjacent includes moves that fill liberties on the last move's chain.
Move Board::GetRandomMoveLocal(int c, MoveInfo urgent, RandomGenerator& gen) const
{
    assert(c != PassCoord);
    std::vector<Move> localMoves;

    // Use the "old" method for now.
    const Point& pt = _points[c];
    int m;
    BitIterator it(*pt.Orthogonals);
    while ((m = it.Next()) != BitIterator::NoBit)
    {
        // Test this location.
        MoveInfo info = CheckMove(m);
        if ((info & Legal) && (info & urgent))
        {
            localMoves.push_back({_colourToMove, m, info});
        }
    }

    // "New" method which crashes sometimes...
  //const StoneChain& chain = _chains[_points[lastCoord].ChainId];
  //FindLegalLibertyMoves(chain, localMoves, urgent);

    return localMoves.empty() ? BadMove : localMoves[gen.Next(localMoves.size())];
}

// Find the legal moves that are adjacent to the specified chain.
void Board::FindLegalLibertyMoves(const StoneChain& chain, std::vector<Move>& moves, MoveInfo urgent) const
{
    BitSet libs(*chain.Neighbours);
    libs &= *_empty;

    int bit;
    BitIterator it(libs);
    while ((bit = it.Next()) != BitIterator::NoBit)
    {
        MoveInfo info = CheckMove(bit);
        if (info & Legal)
        {
            if (urgent == 0 || (info & urgent))
            {
                moves.push_back({_colourToMove, bit, info});
            }
        }
    }
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
        pt.Col = move.Col;

        // Iterate through the chains which are affected by this move.
        std::vector<int> neighbourChains, enemyChains;
        for (Point* const n : pt.Neighbours)
        {
            int nc = n->ChainId;
            if (nc != NoChain)
            {
                const StoneChain& c = _chains[nc];
                if (n->Col != move.Col && c.Liberties == 1)
                {
                    // Capture this enemy group.
                    nextHash ^= CaptureChain(nc);
                }
                else if (n->Col == move.Col)
                {
                    // Friendly chain - need to merge.
                    if (std::find(neighbourChains.begin(), neighbourChains.end(), nc) == neighbourChains.end())
                        neighbourChains.push_back(nc);
                }
                else if (n->Col != None)
                {
                    // Enemy chain.
                    if (std::find(enemyChains.begin(), enemyChains.end(), nc) == enemyChains.end())
                        enemyChains.push_back(nc);
                }
            }
        }

        uint64_t moveHash  = z->Key(move.Col, move.Coord);
        nextHash ^= moveHash;

        // Update the cached BitSets.
        BitSet* friendly = move.Col == Black ? _blackStones : _whiteStones;
        friendly->Set(move.Coord);
        _empty->UnSet(move.Coord);

        // If there are friendly neighbouring chains then combine them.
        CombineChainsForMove(move, moveHash, neighbourChains, enemyChains);
    }

    // Update the colour to move.
    if (move.Col == _colourToMove)
    {
        _colourToMove = _colourToMove == Black ? White : Black;
        nextHash ^= CurrentRules.Ko == Situational ? z->BlackTurn() : 0;
    }

    _hashes.push_back(nextHash);
    _passes[(int)move.Col-1] = move.Coord == PassCoord;
    _lastMove = move;
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
        if (pt.Col == None)
        {
            // Look at a neighbour.
            Point const* const n = pt.Neighbours[0];
            score += n->Col == Black ? 1 : -1;
        }
        else
        {
            score += pt.Col == Black ? 1 : -1;
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
            col = _points[r*_boardSize+c].Col;
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
        _points[i] = { None, i, {}, nullptr, nullptr, NoChain };

    InitialiseNeighbours();
    _hashes.push_back(CurrentRules.Ko == Situational ? Zobrist::Instance()->BlackTurn() : 0);
}

// Initialise the neighbours for each point.
void Board::InitialiseNeighbours()
{
    int r, c;
    for (int i = 0; i < _boardArea; i++)
    {
        r = i / _boardSize, c = i % _boardSize;
        _points[i].Coord = i;

        _points[i].Neighbours.clear();
        _points[i].Orthogonals = new BitSet(_boardArea);
        _points[i].Diagonals = new BitSet(_boardArea);

        // Setup the orthogonals.
        // Also fill in the neighbours vector at this point.
        if (r > 0)
        {
            _points[i].Neighbours.push_back(&_points[i-_boardSize]);
            _points[i].Orthogonals->Set(i-_boardSize);
        }

        if (r < _boardSize-1)
        {
            _points[i].Neighbours.push_back(&_points[i+_boardSize]);
            _points[i].Orthogonals->Set(i+_boardSize);
        }

        if (c > 0)
        {
            _points[i].Neighbours.push_back(&_points[i-1]);
            _points[i].Orthogonals->Set(i-1);
        }

        if (c < _boardSize-1)
        {
            _points[i].Neighbours.push_back(&_points[i+1]);
            _points[i].Orthogonals->Set(i+1);
        }

        // Setup the diagonals.
        if (r > 0 && c > 0) _points[i].Diagonals->Set(i-1-_boardSize);
        if (r > 0 && c < _boardSize-1) _points[i].Diagonals->Set(i+1-_boardSize);
        if (r < _boardSize-1 && c > 0) _points[i].Diagonals->Set(i-1+_boardSize);
        if (r < _boardSize-1 && c < _boardSize-1) _points[i].Diagonals->Set(i+1+_boardSize);
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

int Board::CountChainLiberties(int id) const
{
    const StoneChain& chain = _chains[id];
    return chain.Neighbours->CountAnd(*_empty);
}

int Board::CountChainSize(int id) const
{
    const StoneChain& chain = _chains[id];
    return chain.Stones->Count();
}

uint64_t Board::CaptureChain(int id)
{
    StoneChain& chain = _chains[id];
    int bit;
    BitIterator it(*chain.Stones);
    while ((bit = it.Next()) != BitIterator::NoBit)
    {
        Point& pt = _points[bit];
        pt.Col = None;
        pt.ChainId = NoChain;

        _blackStones->UnSet(bit);
        _whiteStones->UnSet(bit);
        _empty->Set(bit);
    }

    chain.Ignore = true;

    // TODO: Update liberties of neighbouring chains.
    for (size_t i = 0; i < _chains.size(); i++)
    {
        StoneChain& c = _chains[i];
        if (!c.Ignore)
        {
            c.Liberties = c.Neighbours->CountAnd(*_empty);
        }
    }

    return chain.Hash;
}

void Board::CreateNewChainForMove(const Move& move, uint64_t moveHash)
{
    Point& pt = _points[move.Coord];

    BitSet* neighbours = new BitSet(*pt.Orthogonals);
    BitSet* stones = new BitSet(_boardArea);
    stones->Set(move.Coord);

    StoneChain c = { move.Col, neighbours->CountAnd(*_empty), stones, neighbours, moveHash, false };
    _chains.push_back(c);

    pt.ChainId = _chains.size()-1;
}

void Board::CombineChainsForMove(const Move& move, uint64_t moveHash, const std::vector<int>& neighbourChains, const std::vector<int>& enemyChains)
{
    // Reduce the liberties of the enemy chains.
    for (int i : enemyChains)
    {
        StoneChain& c = _chains[i];
        --c.Liberties;
    }

    if (neighbourChains.empty())
    {
        CreateNewChainForMove(move, moveHash);
    }
    else
    {
        Point& pt = _points[move.Coord];

        // Combine everything onto the first neighbour chain.
        int nc = neighbourChains.front();
        StoneChain& base = _chains[nc];

        // Add the newly placed stone.
        base.Stones->Set(move.Coord);
        base.Neighbours->Set(*pt.Orthogonals);
        base.Hash ^= moveHash;
        pt.ChainId = nc;

        // Add any other neighbouring chains.
        for (size_t i = 1; i < neighbourChains.size(); i++)
        {
            StoneChain& n = _chains[neighbourChains[i]];

            // Update stones & neighbours.
            base.Stones->Set(*n.Stones);
            base.Neighbours->Set(*n.Neighbours);
            base.Hash ^= n.Hash;

            // Change the labels on the stones in the neighbour chain.
            int bit;
            BitIterator it(*n.Stones);
            while ((bit = it.Next()) != BitIterator::NoBit)
            {
                Point& pt = _points[bit];
                pt.ChainId = nc;
            }

            n.Ignore = true;
        }

        // Remove potential overlap of neighbours and stones.
        base.Neighbours->UnSet(*base.Stones);

        // Reassess the liberties.
        base.Liberties = base.Neighbours->CountAnd(*_empty);
    }
}

void Board::LogPointDetails(int coord) const
{
    const Point& pt = _points[coord];
    std::cout << pt.Coord << " " << pt.Col << " " << pt.ChainId << std::endl;
    if (pt.ChainId != NoChain)
    {
        const StoneChain& c = _chains[pt.ChainId];
        std::cout << "Liberties: " << c.Liberties << std::endl;
        std::cout << "Hash: " << c.Hash << std::endl;
        std::cout << "Ignore: " << c.Ignore << std::endl;
    }
}
