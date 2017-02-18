#include "ofd/Layer.h"
#include "ofd/Page.h"
#include "utils/logger.h"

using namespace ofd;
using namespace utils;

Layer::Layer(PagePtr page) :
    ID(0), Type(LayerType::BODY),
    m_page(page){
}

Layer::~Layer(){
}

