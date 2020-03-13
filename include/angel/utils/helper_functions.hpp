#pragma once

#include <kitty/dynamic_truth_table.hpp>
#include <kitty/operations.hpp>
#include <angel/utils/partial_truth_table.hpp>

namespace angel
{

inline std::vector<uint32_t> initialize_orders(uint32_t n)
{
    std::vector<uint32_t> orders_init;
    for (int32_t i = n-1; i >= 0; i--)
        orders_init.emplace_back(i);
    return orders_init;
}

inline void extract_independent_vars (std::vector<uint32_t> &zero_lines, std::vector<uint32_t> &one_lines, 
kitty::dynamic_truth_table const& tt)
{       
    /* extract minterms */
    std::vector<partial_truth_table> minterms = on_set( tt );

    /* convert minterms to column vectors */
    uint32_t const minterm_length = minterms[0u].num_bits();
    uint32_t const num_minterms = minterms.size();

    std::vector<partial_truth_table> columns( minterm_length, partial_truth_table( num_minterms ) );
    for ( auto i = 0u; i < minterm_length; ++i )
    {
        for ( auto j = 0u; j < num_minterms; ++j )
        {
            if ( minterms.at( j ).get_bit( i ) )
            {
                columns[minterm_length-i-1].set_bit( j );
            }
        }
    }

    for(int32_t i=minterm_length-1; i>=0; i--)
    {
        if(columns.at(i).is_const0())
        {
            zero_lines.emplace_back(i);
        }
            
        else if(columns.at(i).is_const1())
        {
            one_lines.emplace_back(i);
        }
    }
}

inline std::vector<uint32_t> reordering_on_tt_inplace (kitty::dynamic_truth_table &tt, std::vector<uint32_t> orders)
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

} /// namespace angel end