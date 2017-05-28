#include "Board.h"

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
        delete[] _points;
        _points = nullptr;
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

    for (int i = 0; i < _boardArea; i++)
    {
        Point& pt = _points[i];
        const Point& opt = other._points[i];
        pt.Col = opt.Col;
        pt.GroupSize = opt.GroupSize;
        pt.Liberties = opt.Liberties;
    }
}

// Check the legality of the specified move in this position.
Legality Board::CheckLegal(int loc) const
{
    return CheckLegal(this->_colourToMove, loc);
}

// Check the legality of the specified move in this position.
Legality Board::CheckLegal(Colour col, int loc) const
{
    // Passing is always legal.
    if (loc == PassCoord)
        return Legal;

    // Check occupancy.
    const Point& pt = this->_points[loc];
    Legality res = pt.Col == None ? Legal : Occupied;
    if (res == Legal)
    {
        // Check for suicide and ko.
        int liberties = 0;
        int captureLoc;
        int capturesWithRepetition = 0; // Note: captured neighbours could be in the same group!
        for (const Point* const n : pt.Neighbours)
        {
            if (n->Col == None)
            {
                ++liberties;
            }
            else if (n->Col == col)
            {
                liberties += n->Liberties-1;
            }
            else
            {
                if (n->Liberties == 1)
                {
                    ++liberties;
                    captureLoc = n->Coord;
                    capturesWithRepetition += n->GroupSize;
                }
            }
        }

        bool suicide = liberties == 0;
        bool repetition = capturesWithRepetition == 1 && IsKoRepetition(col, loc, captureLoc);
        res = suicide ? Suicide : repetition ? Ko : Legal;
    }

    return res;
}

// Check whether the specified move will fill an eye.
// This is currently more of a pseudo-eye check as it does not look at the diagonals.
bool Board::FillsEye(Colour col, int loc) const
{
    size_t friendlyNeighbours = 0;
    const Point& pt = this->_points[loc];
    for (const Point* const n : pt.Neighbours)
    {
        friendlyNeighbours += n->Col == col && n->Liberties > 1 ? 1 : 0;
    }

    return friendlyNeighbours == pt.Neighbours.size();
}

// Get all moves available for the current colour.
std::vector<Move> Board::GetMoves(bool duringPlayout) const
{
    std::vector<Move> moves;
    if (!GameOver())
    {
        for (int i = 0; i < _boardArea; i++)
        {
            if (CheckLegal(i) == Legal)
            {
                if (!duringPlayout || !FillsEye(this->_colourToMove, i))
                {
                    moves.push_back({this->_colourToMove, i});
                }
            }
        }

        // Passing is always legal but don't consider it during a playout unless there
        // are no other moves available.
        if (!duringPlayout || moves.size() == 0)
        {
            moves.push_back({this->_colourToMove, PassCoord});
        }
    }

    return moves;
}

// Update the board state with the specified move.
void Board::MakeMove(const Move& move)
{
    assert(!GameOver());
    assert(CheckLegal(move.Col, move.Coord) == Legal);

    auto z = Zobrist::Instance();
    uint64_t nextHash = _hashes[_turnNumber-1];

    if (move.Coord != PassCoord)
    {
        nextHash ^= z->Key(move.Col, move.Coord);

        Point& pt = this->_points[move.Coord];
        pt.Col = move.Col;

        // Cache which points need updating.
        bool requireUpdate[_boardArea] = {false};
        requireUpdate[pt.Coord] = true;

        // First detect stones which will get captured and remove them.
        for (Point* const n : pt.Neighbours)
        {
            if (n->Col != None)
            {
                if (n->Col != move.Col && n->Liberties == 1)
                {
                    uint64_t groupHash = 0;
                    FFCapture(n, requireUpdate, groupHash);
                    nextHash ^= groupHash;
                }
                else
                {
                    requireUpdate[n->Coord] = true;
                }
            }
        }

        // Update the liberties of the affected stones.
        for (int i = 0; i < _boardArea; i++)
        {
            if (requireUpdate[i])
            {
                int liberties = 0;
                std::vector<Point*> group;
                std::vector<Point*> considered;
                FFLiberties(&_points[i], requireUpdate, liberties, group, considered);
            }
        }
    }

    // Update the colour to move.
    if (move.Col == this->_colourToMove)
    {
        this->_colourToMove = this->_colourToMove == Black ? White : Black;
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
    _points = new Point[_boardArea];
    for (int i = 0; i < _boardArea; i++)
        _points[i] = { None, 0, 0 };

    this->InitialiseNeighbours();
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
        if (r > 0) _points[i].Neighbours.push_back(&_points[i-_boardSize]);
        if (r < _boardSize-1) _points[i].Neighbours.push_back(&_points[i+_boardSize]);
        if (c > 0) _points[i].Neighbours.push_back(&_points[i-1]);
        if (c < _boardSize-1) _points[i].Neighbours.push_back(&_points[i+1]);
    }
}

// Check whether the specified move and capture would result in a board repetition.
bool Board::IsKoRepetition(Colour col, int loc, int captureLoc) const
{
    Colour enemyCol = col == Black ? White : Black;
    auto z = Zobrist::Instance();
    uint64_t nextHash =
        _hashes[_turnNumber-1]
        ^ z->Key(col, loc)
        ^ z->Key(enemyCol, captureLoc)
        ^ (CurrentRules.Ko == Situational ? z->BlackTurn() : 0);

    // Has this hash occurred previously?
    bool repeat = false;
    for (int i = _turnNumber-1; i >= 0 && !repeat; i--)
        repeat = _hashes[i] == nextHash;

    return repeat;
}

// Flood fill algorithm which updates the liberties for all stones in the group containing
// the specified point.
void Board::FFLiberties(Point* const pt, bool* requireUpdate, int& liberties, std::vector<Point*>& group, std::vector<Point*>& considered, bool root)
{
    group.push_back(pt);
    for (Point* const n : pt->Neighbours)
    {
        if (n->Col == pt->Col &&
            std::find(group.begin(), group.end(), n) == group.end())
        {
            FFLiberties(n, requireUpdate, liberties, group, considered, false);
        }
        else if (n->Col == None &&
            std::find(considered.begin(), considered.end(), n) == considered.end())
        {
            ++liberties;
            considered.push_back(n);
        }
    }

    if (root)
    {
        for (Point* const groupPt : group)
        {
            groupPt->Liberties = liberties;
            groupPt->GroupSize = group.size();
            requireUpdate[groupPt->Coord] = false;
        }
    }
}

// Flood fill algorithm which removes all stones in the group containing the specified point.
// Points which are affected by this capture get flagged as requiring an update.
void Board::FFCapture(Point* const pt, bool* requireUpdate, uint64_t& groupHash)
{
    Colour origCol = pt->Col;
    pt->Col = None;
    pt->Liberties = 0;
    groupHash ^= Zobrist::Instance()->Key(origCol, pt->Coord);

    for (Point* const n : pt->Neighbours)
    {
        if (n->Col == origCol)
        {
            FFCapture(n, requireUpdate, groupHash);
        }
        else if (n->Col != None)
        {
            requireUpdate[n->Coord] = true;
        }
    }
}
