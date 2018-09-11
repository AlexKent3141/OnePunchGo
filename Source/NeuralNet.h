#ifndef __NEURAL_NET_H__
#define __NEURAL_NET_H__

extern "C"
{
    #include "darknet/darknet.h"
}

#include "Args.h"
#include "Board.h"
#include <mutex>
#include <vector>

// This singleton class wraps up the neural network(s).
class NeuralNet
{
public:
    static void InitialiseNets(size_t numNets)
    {
        // Check whether a selection/value weights directory has been specified.
        std::string selectionFolder = "./selection_weights";
        std::string valueFolder = "./value_weights";

        auto args = Args::Get();
        args->TryParse("-sel", selectionFolder);
        args->TryParse("-val", valueFolder);

        const std::string& selConfigPath = selectionFolder + "/sel.cfg";
        const std::string& selWeightsPath = selectionFolder + "/sel.weights";
        for (size_t i = _selNets.size(); i < numNets; i++)
            _selNets.push_back(new NeuralNet(selConfigPath, selWeightsPath));

        const std::string& valConfigPath = valueFolder + "/val.cfg";
        const std::string& valWeightsPath = valueFolder + "/val.weights";
        for (size_t i = _valNets.size(); i < numNets; i++)
            _valNets.push_back(new NeuralNet(valConfigPath, valWeightsPath));
    }

    static NeuralNet* GetSelectionNetwork(size_t id)
    {
        assert(id < _selNets.size());
        return _selNets[id];
    }

    static NeuralNet* GetValueNetwork(size_t id)
    {
        assert(id < _valNets.size());
        return _valNets[id];
    }

    ~NeuralNet()
    {
        if (_nn != nullptr)
        {
            free_network(_nn);
            _nn = nullptr;
        }
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
    network* _nn = nullptr;
    static std::vector<NeuralNet*> _selNets;
    static std::vector<NeuralNet*> _valNets;

    NeuralNet(const std::string& net, const std::string& weights)
    {
        _nn = load_network((char*)net.c_str(), (char*)weights.c_str(), 0);
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
