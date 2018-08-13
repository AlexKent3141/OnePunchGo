#include "NeuralNet.h"

NeuralNet* NeuralNet::_selection = nullptr;
std::mutex* NeuralNet::_msel = new std::mutex();
