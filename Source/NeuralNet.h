#ifndef __NEURAL_NET_H__
#define __NEURAL_NET_H__

extern "C"
{
    #include "darknet/darknet.h"
}

#include "Board.h"
#include <mutex>

// This singleton class wraps up the neural network(s).
class NeuralNet
{
public:
    ~NeuralNet()
    {
        if (_nn != nullptr)
        {
            free_network(_nn);
            _nn = nullptr;
        }
    }

    // Use the selection network to prioritise the moves for this board state.
    static std::vector<double> Select(const Board& board, double& total)
    {
        std::unique_lock<std::mutex> lk(*_msel);
        if (_selection == nullptr)
        {
            _selection = new NeuralNet("sel.cfg", "sel.weights");
        }

        return _selection->Evaluate(board, total);
    }

    std::vector<double> Evaluate(const Board& board, double& total)
    {
        assert(board.Size() == 19);
        float* inputs = GetInputs(board);
        float* eval = network_predict(_nn, inputs);
        delete[] inputs;

        // Normalise the outputs.
        float largest = 0;
        for (int i = 0; i < 361; i++)
        {
            if (eval[i] > largest)
            {
                largest = eval[i];
            }
        }

        assert(largest > 0);

        total = 0;
        double out;
        std::vector<double> outputs;
        outputs.reserve(361);
        for (int i = 0; i < 361; i++)
        {
            out = eval[i] / largest;
            total += out;
            outputs.push_back(out);
        }

        return outputs;
    }

private:
    // The move selection.
    static NeuralNet* _selection;
    static std::mutex* _msel;

    network* _nn = nullptr;

    NeuralNet(const char* net, const char* weights)
    {
        _nn = load_network((char*)net, (char*)weights, 0);
    }

    float* GetInputs(const Board& board) const
    {
        // Construct the input planes for the network.
        float* input = new float[3*361];
        memset(input, 0, 3*361*sizeof(float));

        Colour ptm = board.ColourToMove();
        for (int i = 0; i < 361; i++)
        {
            Colour ptCol = board.PointColour(i);
            if (ptCol == ptm) // Stone for the current player.
                input[i] = 1;
            else if (ptCol != None) // Stone for the opposing player.
                input[i+361] = 1;
            else if (board.CheckMove(i) & Legal) // Legal move for the current player.
                input[i+2*361] = 1;
        }
        
        return input;
    }
};

#endif // __NEURAL_NET_H__
