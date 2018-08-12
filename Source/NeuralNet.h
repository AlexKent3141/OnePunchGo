#ifndef __NEURAL_NET_H__
#define __NEURAL_NET_H__

extern "C"
{
    #include "darknet/darknet.h"
}

#include "Board.h"
#include <iostream>

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
    static std::vector<double> Select(const Board& board)
    {
        if (_selection == nullptr)
        {
            _selection = new NeuralNet("sel.cfg", "sel.weights");
        }

        return _selection->Evaluate(board);
    }

    std::vector<double> Evaluate(const Board& board)
    {
        std::cout << "Evaluating board" << std::endl;
        assert(board.Size() == 19);
        float* inputs = GetInputs(board);
        float* eval = network_predict(_nn, inputs);
        std::cout << "Predicted" << std::endl;
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

        std::cout << "Largest: " << largest << std::endl;
        assert(largest > 0);

        std::vector<double> outputs;
        outputs.reserve(361);
        for (int i = 0; i < 361; i++)
        {
            outputs.push_back(eval[i] / largest);
        }

        return outputs;
    }

private:
    // The move selection.
    static NeuralNet* _selection;

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
