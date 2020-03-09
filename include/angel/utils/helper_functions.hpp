#pragma once

#include <kitty/dynamic_truth_table.hpp>
#include <kitty/operations.hpp>

namespace angel
{

std::vector<uint32_t> initialize_orders(uint32_t n)
{
    std::vector<uint32_t> orders_init;
    for (int32_t i = n-1; i >= 0; i--)
        orders_init.emplace_back(i);
    return orders_init;
}


std::vector<uint32_t> reordering_on_tt_inplace (kitty::dynamic_truth_table &tt, std::vector<uint32_t> orders)
{
    auto var_num = orders.size();
    std::vector<uint32_t> new_order;
    std::reverse(orders.begin(), orders.end());
    
    for(auto i=0; i<var_num; i++)
    {
        if(i != orders[i])
        {
            for(auto j=i+1; j<var_num; j++)
            {
                if(j == orders[i])
                {
                    kitty::swap_inplace(tt, i, j); 
                    new_order.emplace_back(j);
                    break;
                }
            }
        }
        else
        {
            new_order.emplace_back(i);
        }
        
    }
    return new_order;
}

} /* end namespace angel */