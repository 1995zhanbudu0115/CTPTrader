#include "ctastrategybase.h"
#include "ctaengine.h"

StrategyBase::StrategyBase(CtaEngine *ce, std::string name, std::string symbol)
: ce(ce), name(name), vtSymbol(symbol)
{

}

int StrategyBase::convert_time_str2int(const char *update_time)
{
    //remove_if(beg, end, op)   //移除区间[beg,end)中每一个“令判断式:op(elem)获得true”的元素；
    std::string market_time_str = update_time;
    market_time_str.erase(std::remove_if(market_time_str.begin(), market_time_str.end(), [](char c){ return c == ':'; })
                          , market_time_str.end());
    return atoi(market_time_str.c_str());
}

std::string StrategyBase::buy(double price, int volume, bool stop)
{
    return sendOrder(CTAORDER_BUY, price, volume, stop);//买多，开多
}

std::string StrategyBase::sell(double price, int volume, bool stop)
{
    return sendOrder(CTAORDER_SELL, price, volume, stop); //卖平，平空
}

std::string StrategyBase::short_(double price, int volume, bool stop)
{
    return sendOrder(CTAORDER_SHORT, price, volume, stop);  // 卖开，开空
}

std::string StrategyBase::cover(double price, int volume, bool stop)
{
    return sendOrder(CTAORDER_COVER, price, volume, stop);  // 买平，平多
}

void StrategyBase::cancelOrder(std::string orderID)
{
    if (orderID.empty())
    {
        return;
    }

    if (std::string::npos != orderID.find_first_of(STOPORDERPREFIX))
    {
        // 取消停止单
        ce->cancelStopOrder(orderID);
    }
    else
    {
        ce->cancelOrder(orderID);
    }
}

std::string StrategyBase::sendOrder(char order_type, double price, int volume, bool stop)
{
    std::string orderID;
    if (trading)
    {
        // 如果stop为True，则意味着发本地停止单
        if (stop)
        {
            orderID = ce->sendStopOrder(vtSymbol, order_type, price, volume, std::shared_ptr<StrategyBase>(this));
        }
        else
        {
            orderID = ce->sendOrder(vtSymbol, order_type, price, volume, std::shared_ptr<StrategyBase>(this));
        }
    }

    return orderID;
}


