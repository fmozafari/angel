#include "no_deps.hpp"
#include <kitty/dynamic_truth_table.hpp>
#include <angel/utils/partial_truth_table.hpp>
#include <angel/quantum_state_preparation/qsp_tt_general.hpp>

namespace angel
{

class ResubSynthesisDeps : public NoDeps
{
public:
    ResubSynthesisDeps() 
    {
        considering_deps = true;
    }

    /*---chech that there isn't any dependency ----*/
    bool check_not_exist_dependencies( std::vector<partial_truth_table> minterms, uint32_t target )
    {
        uint32_t const num_minterms = minterms.size();

        for ( auto i = 0 ; i < int32_t( minterms.size() ) ; i++ )
        {
            for(int32_t j =target-1 ; j>=0 ; j--)
            {
                minterms[i].clear_bit(j);
            }
        }

        for(auto k = 0u ; k<num_minterms-1 ; k++)
        {
            auto row1 = minterms[k];
            auto row2 = minterms[k+1];
            auto check = row1^row2;

            if ( check.count_ones()==1 && check.get_bit( target ) )
            {
            return true;
            }
        }

        return false;
    }

    dependencies_t run(kitty::dynamic_truth_table const &tt, qsp_general_stats &stats)
    {
        ++stats.total_bench;

        /* extract minterms */
        std::vector<partial_truth_table> minterms = on_set(tt);

        /* convert minterms to column vectors */
        uint32_t const minterm_length = minterms[0u].num_bits();
        uint32_t const num_minterms = minterms.size();

        std::vector<partial_truth_table> columns(minterm_length, partial_truth_table(num_minterms));
        for (auto i = 0u; i < minterm_length; ++i)
        {
            for (auto j = 0u; j < num_minterms; ++j)
            {
                if (minterms.at(j).get_bit(i))
                {
                    columns[i/*minterm_length - i - 1*/].set_bit(j);
                }
            }
        }

        /* resubstitution-style dependency analysis */
        dependencies_t dependencies;

        /* check there is no deps */
        uint32_t has_no_dependencies = 0u;

        for (auto mirror_i = 0; mirror_i < int32_t(columns.size()); ++mirror_i)
        {
            auto i = columns.size() - mirror_i - 1;

            /*---check that there isn't any dependency---*/
            if (i < minterm_length - 2)
            {
                if (check_not_exist_dependencies(minterms, i))
                {
                    ++has_no_dependencies;
                    continue;
                }
            }
        }

        if (has_no_dependencies == (minterm_length - 2u))
        {
            ++stats.has_no_dependencies;
            return dependencies;
        }

        for (auto mirror_i = 0; mirror_i < int32_t(columns.size()); ++mirror_i)
        {
            auto i = columns.size() - mirror_i - 1;
            if (columns.at(i).is_const())
                continue;

            bool found = false;
            for (auto j = uint32_t(columns.size()) - 1; j > i; --j)
            {
                if (columns.at(j).is_const())
                    continue;

                if (columns.at(i) == columns.at(j))
                {
                    found = true;
                    dependencies[i] = std::vector{std::pair{std::string{"eq"}, std::vector<uint32_t>{uint32_t(j * 2 + 0)}}};
                    break;
                }
            }

            /* go to next column if a solution has been found for this column */
            if (found)
                continue;

            for (auto j = uint32_t(columns.size()) - 1; j > i; --j)
            {
                if (columns.at(j).is_const())
                    continue;

                if (columns.at(i) == ~columns.at(j))
                {
                    found = true;
                    dependencies[i] = std::vector{std::pair{std::string{"not"}, std::vector<uint32_t>{uint32_t(j * 2 + 0)}}};
                    break;
                }
            }

            /* go to next column if a solution has been found for this column */
            if (found)
                continue;

            /*-----xor---------*/
            /*----first input */
            for (auto j = uint32_t(columns.size()) - 1; j > i; --j)
            {
                if (columns.at(j).is_const())
                    continue;

                /*-----second input */
                for (auto k = j - 1; k > i; --k)
                {
                    if (columns.at(k).is_const())
                        continue;

                    if (columns.at(i) == (columns.at(j) ^ columns.at(k)))
                    {
                        found = true;
                        dependencies[i] = std::vector{std::pair{std::string{"xor"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0)}}};
                        break;
                    }
                    else if (columns.at(i) == (~(columns.at(j) ^ columns.at(k))))
                    {
                        found = true;
                        dependencies[i] = std::vector{std::pair{std::string{"xnor"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0)}}};
                        break;
                    }

                    /*-----3rd input */
                    for (auto l = j - 2; l > i; --l)
                    {
                        if (columns.at(l).is_const())
                            continue;

                        if (columns.at(i) == (columns.at(j) ^ columns.at(k) ^ columns.at(l)))
                        {
                            found = true;
                            dependencies[i] = std::vector{std::pair{std::string{"xor"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0)}}};
                            break;
                        }
                        else if (columns.at(i) == (~(columns.at(j) ^ columns.at(k) ^ columns.at(l))))
                        {
                            found = true;
                            dependencies[i] = std::vector{std::pair{std::string{"xnor"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0)}}};
                            break;
                        }
                        /*---- 4th input */
                        for (auto m = j - 3; m > i; --m)
                        {
                            if (columns.at(m).is_const())
                                continue;

                            if (columns.at(i) == (columns.at(j) ^ columns.at(k) ^ columns.at(l) ^ columns.at(m)))
                            {
                                found = true;
                                dependencies[i] = std::vector{std::pair{std::string{"xor"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0), uint32_t(m * 2 + 0)}}};
                                break;
                            }
                            else if (columns.at(i) == (~(columns.at(j) ^ columns.at(k) ^ columns.at(l) ^ columns.at(m))))
                            {
                                found = true;
                                dependencies[i] = std::vector{std::pair{std::string{"xnor"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0), uint32_t(m * 2 + 0)}}};
                                break;
                            }

                            /*---- 5th input */
                            for (auto i5 = j - 4; i5 > i; --i5)
                            {
                                if (columns.at(i5).is_const())
                                    continue;

                                if (columns.at(i) == (columns.at(j) ^ columns.at(k) ^ columns.at(l) ^ columns.at(m) ^ columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"xor"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0), uint32_t(m * 2 + 0), uint32_t(i5 * 2 + 0)}}};
                                    break;
                                }
                                else if (columns.at(i) == (~(columns.at(j) ^ columns.at(k) ^ columns.at(l) ^ columns.at(m) ^ columns.at(i5))))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"xnor"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0), uint32_t(m * 2 + 0), uint32_t(i5 * 2 + 0)}}};
                                    break;
                                }
                            }
                            if (found)
                                break;
                        }
                        if (found)
                            break;
                    }
                    if (found)
                        break;
                }
                if (found)
                    break;
            }

            if (found)
                continue;

            /*-----and---------*/
            /*-----first input */
            for (auto j = uint32_t(columns.size()) - 1; j > i; --j)
            {
                if (columns.at(j).is_const())
                    continue;

                //----second input
                for (auto k = j - 1; k > i; --k)
                {
                    if (columns.at(k).is_const())
                        continue;

                    if (columns.at(i) == (columns.at(j) & columns.at(k)))
                    {
                        found = true;
                        dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0)}}};
                        break;
                    }
                    else if (columns.at(i) == (~(columns.at(j) & columns.at(k))))
                    {
                        found = true;
                        dependencies[i] = std::vector{std::pair{std::string{"nand"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0)}}};
                        break;
                    }
                    else if (columns.at(i) == (~columns.at(j) & columns.at(k)))
                    {
                        found = true;
                        dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 0)}}};
                        break;
                    }
                    else if (columns.at(i) == (columns.at(j) & ~columns.at(k)))
                    {
                        found = true;
                        dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 1)}}};
                        break;
                    }
                    else if (columns.at(i) == (~columns.at(j) & ~columns.at(k)))
                    {
                        found = true;
                        dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 1)}}};
                        break;
                    }

                    /*----3rd input */
                    for (auto l = j - 2; l > i; --l)
                    {
                        if (columns.at(l).is_const())
                            continue;

                        if (columns.at(i) == (columns.at(j) & columns.at(k) & columns.at(l)))
                        {
                            found = true;
                            dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0)}}};
                            break;
                        }
                        else if (columns.at(i) == (~(columns.at(j) & columns.at(k) & columns.at(l))))
                        {
                            found = true;
                            dependencies[i] = std::vector{std::pair{std::string{"nand"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0)}}};
                            break;
                        }
                        else if (columns.at(i) == (~columns.at(j) & columns.at(k) & columns.at(l)))
                        {
                            found = true;
                            dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0)}}};
                            break;
                        }
                        else if (columns.at(i) == (columns.at(j) & ~columns.at(k) & columns.at(l)))
                        {
                            found = true;
                            dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 1), uint32_t(l * 2 + 0)}}};
                            break;
                        }
                        else if (columns.at(i) == (~columns.at(j) & ~columns.at(k) & columns.at(l)))
                        {
                            found = true;
                            dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 1), uint32_t(l * 2 + 0)}}};
                            break;
                        }
                        else if (columns.at(i) == (columns.at(j) & columns.at(k) & ~columns.at(l)))
                        {
                            found = true;
                            dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0), uint32_t(l * 2 + 1)}}};
                            break;
                        }
                        else if (columns.at(i) == (~columns.at(j) & columns.at(k) & ~columns.at(l)))
                        {
                            found = true;
                            dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 0), uint32_t(l * 2 + 1)}}};
                            break;
                        }
                        else if (columns.at(i) == (columns.at(j) & ~columns.at(k) & ~columns.at(l)))
                        {
                            found = true;
                            dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 1), uint32_t(l * 2 + 1)}}};
                            break;
                        }
                        else if (columns.at(i) == (~columns.at(j) & ~columns.at(k) & ~columns.at(l)))
                        {
                            found = true;
                            dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 1), uint32_t(l * 2 + 1)}}};
                            break;
                        }

                        /*---- 4th input */
                        for (auto m = j - 3; m > i; --m)
                        {
                            if (columns.at(m).is_const())
                                continue;

                            if (columns.at(i) == (columns.at(j) & columns.at(k) & columns.at(l) & columns.at(m)))
                            {
                                found = true;
                                dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0), uint32_t(m * 2 + 0)}}};
                                break;
                            }
                            else if (columns.at(i) == (~(columns.at(j) & columns.at(k) & columns.at(l) & columns.at(m))))
                            {
                                found = true;
                                dependencies[i] = std::vector{std::pair{std::string{"nand"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0), uint32_t(m * 2 + 0)}}};
                                break;
                            }
                            else if (columns.at(i) == (~columns.at(j) & columns.at(k) & columns.at(l) & columns.at(m)))
                            {
                                found = true;
                                dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0), uint32_t(m * 2 + 0)}}};
                                break;
                            }
                            else if (columns.at(i) == (columns.at(j) & ~columns.at(k) & columns.at(l) & columns.at(m)))
                            {
                                found = true;
                                dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 1), uint32_t(l * 2 + 0), uint32_t(m * 2 + 0)}}};
                                break;
                            }
                            else if (columns.at(i) == (~columns.at(j) & ~columns.at(k) & columns.at(l) & columns.at(m)))
                            {
                                found = true;
                                dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 1), uint32_t(l * 2 + 0), uint32_t(m * 2 + 0)}}};
                                break;
                            }
                            else if (columns.at(i) == (columns.at(j) & columns.at(k) & ~columns.at(l) & columns.at(m)))
                            {
                                found = true;
                                dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0), uint32_t(l * 2 + 1), uint32_t(m * 2 + 0)}}};
                                break;
                            }
                            else if (columns.at(i) == (~columns.at(j) & columns.at(k) & ~columns.at(l) & columns.at(m)))
                            {
                                found = true;
                                dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 0), uint32_t(l * 2 + 1), uint32_t(m * 2 + 0)}}};
                                break;
                            }
                            else if (columns.at(i) == (columns.at(j) & ~columns.at(k) & ~columns.at(l) & columns.at(m)))
                            {
                                found = true;
                                dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 1), uint32_t(l * 2 + 1), uint32_t(m * 2 + 0)}}};
                                break;
                            }
                            else if (columns.at(i) == (~columns.at(j) & ~columns.at(k) & ~columns.at(l) & columns.at(m)))
                            {
                                found = true;
                                dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 1), uint32_t(l * 2 + 1), uint32_t(m * 2 + 0)}}};
                                break;
                            }
                            else if (columns.at(i) == (columns.at(j) & columns.at(k) & columns.at(l) & ~columns.at(m)))
                            {
                                found = true;
                                dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0), uint32_t(m * 2 + 1)}}};
                                break;
                            }
                            else if (columns.at(i) == (~columns.at(j) & columns.at(k) & columns.at(l) & ~columns.at(m)))
                            {
                                found = true;
                                dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0), uint32_t(m * 2 + 1)}}};
                                break;
                            }
                            else if (columns.at(i) == (columns.at(j) & ~columns.at(k) & columns.at(l) & ~columns.at(m)))
                            {
                                found = true;
                                dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 1), uint32_t(l * 2 + 0), uint32_t(m * 2 + 1)}}};
                                break;
                            }
                            else if (columns.at(i) == (~columns.at(j) & ~columns.at(k) & columns.at(l) & ~columns.at(m)))
                            {
                                found = true;
                                dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 1), uint32_t(l * 2 + 0), uint32_t(m * 2 + 1)}}};
                                break;
                            }
                            else if (columns.at(i) == (columns.at(j) & columns.at(k) & ~columns.at(l) & ~columns.at(m)))
                            {
                                found = true;
                                dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0), uint32_t(l * 2 + 1), uint32_t(m * 2 + 1)}}};
                                break;
                            }
                            else if (columns.at(i) == (~columns.at(j) & columns.at(k) & ~columns.at(l) & ~columns.at(m)))
                            {
                                found = true;
                                dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 0), uint32_t(l * 2 + 1), uint32_t(m * 2 + 1)}}};
                                break;
                            }
                            else if (columns.at(i) == (columns.at(j) & ~columns.at(k) & ~columns.at(l) & ~columns.at(m)))
                            {
                                found = true;
                                dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 1), uint32_t(l * 2 + 1), uint32_t(m * 2 + 1)}}};
                                break;
                            }
                            else if (columns.at(i) == (~columns.at(j) & ~columns.at(k) & ~columns.at(l) & ~columns.at(m)))
                            {
                                found = true;
                                dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 1), uint32_t(l * 2 + 1), uint32_t(m * 2 + 1)}}};
                                break;
                            }

                            /*---- 5th input */
                            for (auto i5 = j - 4; i5 > i; --i5)
                            {
                                if (columns.at(i5).is_const())
                                    continue;

                                if (columns.at(i) == (columns.at(j) & columns.at(k) & columns.at(l) & columns.at(m) & columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0), uint32_t(m * 2 + 0), uint32_t(i5 * 2 + 0)}}};
                                    break;
                                }
                                else if (columns.at(i) == (~(columns.at(j) & columns.at(k) & columns.at(l) & columns.at(m) & columns.at(i5))))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"nand"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0), uint32_t(m * 2 + 0), uint32_t(i5 * 2 + 0)}}};
                                    break;
                                }
                                else if (columns.at(i) == (~columns.at(j) & columns.at(k) & columns.at(l) & columns.at(m) & columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0), uint32_t(m * 2 + 0), uint32_t(i5 * 2 + 0)}}};
                                    break;
                                }
                                else if (columns.at(i) == (columns.at(j) & ~columns.at(k) & columns.at(l) & columns.at(m) & columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 1), uint32_t(l * 2 + 0), uint32_t(m * 2 + 0), uint32_t(i5 * 2 + 0)}}};
                                    break;
                                }
                                else if (columns.at(i) == (~columns.at(j) & ~columns.at(k) & columns.at(l) & columns.at(m) & columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 1), uint32_t(l * 2 + 0), uint32_t(m * 2 + 0), uint32_t(i5 * 2 + 0)}}};
                                    break;
                                }
                                else if (columns.at(i) == (columns.at(j) & columns.at(k) & ~columns.at(l) & columns.at(m) & columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0), uint32_t(l * 2 + 1), uint32_t(m * 2 + 0), uint32_t(i5 * 2 + 0)}}};
                                    break;
                                }
                                else if (columns.at(i) == (~columns.at(j) & columns.at(k) & ~columns.at(l) & columns.at(m) & columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 0), uint32_t(l * 2 + 1), uint32_t(m * 2 + 0), uint32_t(i5 * 2 + 0)}}};
                                    break;
                                }
                                else if (columns.at(i) == (columns.at(j) & ~columns.at(k) & ~columns.at(l) & columns.at(m) & columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 1), uint32_t(l * 2 + 1), uint32_t(m * 2 + 0), uint32_t(i5 * 2 + 0)}}};
                                    break;
                                }
                                else if (columns.at(i) == (~columns.at(j) & ~columns.at(k) & ~columns.at(l) & columns.at(m) & columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 1), uint32_t(l * 2 + 1), uint32_t(m * 2 + 0), uint32_t(i5 * 2 + 0)}}};
                                    break;
                                }
                                else if (columns.at(i) == (columns.at(j) & columns.at(k) & columns.at(l) & ~columns.at(m) & columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0), uint32_t(m * 2 + 1), uint32_t(i5 * 2 + 0)}}};
                                    break;
                                }
                                else if (columns.at(i) == (~columns.at(j) & columns.at(k) & columns.at(l) & ~columns.at(m) & columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0), uint32_t(m * 2 + 1), uint32_t(i5 * 2 + 0)}}};
                                    break;
                                }
                                else if (columns.at(i) == (columns.at(j) & ~columns.at(k) & columns.at(l) & ~columns.at(m) & columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 1), uint32_t(l * 2 + 0), uint32_t(m * 2 + 1), uint32_t(i5 * 2 + 0)}}};
                                    break;
                                }
                                else if (columns.at(i) == (~columns.at(j) & ~columns.at(k) & columns.at(l) & ~columns.at(m) & columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 1), uint32_t(l * 2 + 0), uint32_t(m * 2 + 1), uint32_t(i5 * 2 + 0)}}};
                                    break;
                                }
                                else if (columns.at(i) == (columns.at(j) & columns.at(k) & ~columns.at(l) & ~columns.at(m) & columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0), uint32_t(l * 2 + 1), uint32_t(m * 2 + 1), uint32_t(i5 * 2 + 0)}}};
                                    break;
                                }
                                else if (columns.at(i) == (~columns.at(j) & columns.at(k) & ~columns.at(l) & ~columns.at(m) & columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 0), uint32_t(l * 2 + 1), uint32_t(m * 2 + 1), uint32_t(i5 * 2 + 0)}}};
                                    break;
                                }
                                else if (columns.at(i) == (columns.at(j) & ~columns.at(k) & ~columns.at(l) & ~columns.at(m) & columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 1), uint32_t(l * 2 + 1), uint32_t(m * 2 + 1), uint32_t(i5 * 2 + 0)}}};
                                    break;
                                }
                                else if (columns.at(i) == (~columns.at(j) & ~columns.at(k) & ~columns.at(l) & ~columns.at(m) & columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 1), uint32_t(l * 2 + 1), uint32_t(m * 2 + 1), uint32_t(i5 * 2 + 0)}}};
                                    break;
                                }
                                else if (columns.at(i) == (columns.at(j) & columns.at(k) & columns.at(l) & columns.at(m) & ~columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0), uint32_t(m * 2 + 0), uint32_t(i5 * 2 + 1)}}};
                                    break;
                                }
                                else if (columns.at(i) == (~columns.at(j) & columns.at(k) & columns.at(l) & columns.at(m) & ~columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0), uint32_t(m * 2 + 0), uint32_t(i5 * 2 + 1)}}};
                                    break;
                                }
                                else if (columns.at(i) == (columns.at(j) & ~columns.at(k) & columns.at(l) & columns.at(m) & ~columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 1), uint32_t(l * 2 + 0), uint32_t(m * 2 + 0), uint32_t(i5 * 2 + 1)}}};
                                    break;
                                }
                                else if (columns.at(i) == (~columns.at(j) & ~columns.at(k) & columns.at(l) & columns.at(m) & ~columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 1), uint32_t(l * 2 + 0), uint32_t(m * 2 + 0), uint32_t(i5 * 2 + 1)}}};
                                    break;
                                }
                                else if (columns.at(i) == (columns.at(j) & columns.at(k) & ~columns.at(l) & columns.at(m) & ~columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0), uint32_t(l * 2 + 1), uint32_t(m * 2 + 0), uint32_t(i5 * 2 + 1)}}};
                                    break;
                                }
                                else if (columns.at(i) == (~columns.at(j) & columns.at(k) & ~columns.at(l) & columns.at(m) & ~columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 0), uint32_t(l * 2 + 1), uint32_t(m * 2 + 0), uint32_t(i5 * 2 + 1)}}};
                                    break;
                                }
                                else if (columns.at(i) == (columns.at(j) & ~columns.at(k) & ~columns.at(l) & columns.at(m) & ~columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 1), uint32_t(l * 2 + 1), uint32_t(m * 2 + 0), uint32_t(i5 * 2 + 1)}}};
                                    break;
                                }
                                else if (columns.at(i) == (~columns.at(j) & ~columns.at(k) & ~columns.at(l) & columns.at(m) & ~columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 1), uint32_t(l * 2 + 1), uint32_t(m * 2 + 0), uint32_t(i5 * 2 + 1)}}};
                                    break;
                                }
                                else if (columns.at(i) == (columns.at(j) & columns.at(k) & columns.at(l) & ~columns.at(m) & ~columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0), uint32_t(m * 2 + 1), uint32_t(i5 * 2 + 1)}}};
                                    break;
                                }
                                else if (columns.at(i) == (~columns.at(j) & columns.at(k) & columns.at(l) & ~columns.at(m) & ~columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0), uint32_t(m * 2 + 1), uint32_t(i5 * 2 + 1)}}};
                                    break;
                                }
                                else if (columns.at(i) == (columns.at(j) & ~columns.at(k) & columns.at(l) & ~columns.at(m) & ~columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 1), uint32_t(l * 2 + 0), uint32_t(m * 2 + 1), uint32_t(i5 * 2 + 1)}}};
                                    break;
                                }
                                else if (columns.at(i) == (~columns.at(j) & ~columns.at(k) & columns.at(l) & ~columns.at(m) & ~columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 1), uint32_t(l * 2 + 0), uint32_t(m * 2 + 1), uint32_t(i5 * 2 + 1)}}};
                                    break;
                                }
                                else if (columns.at(i) == (columns.at(j) & columns.at(k) & ~columns.at(l) & ~columns.at(m) & ~columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0), uint32_t(l * 2 + 1), uint32_t(m * 2 + 1), uint32_t(i5 * 2 + 1)}}};
                                    break;
                                }
                                else if (columns.at(i) == (~columns.at(j) & columns.at(k) & ~columns.at(l) & ~columns.at(m) & ~columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 0), uint32_t(l * 2 + 1), uint32_t(m * 2 + 1), uint32_t(i5 * 2 + 1)}}};
                                    break;
                                }
                                else if (columns.at(i) == (columns.at(j) & ~columns.at(k) & ~columns.at(l) & ~columns.at(m) & ~columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 1), uint32_t(l * 2 + 1), uint32_t(m * 2 + 1), uint32_t(i5 * 2 + 1)}}};
                                    break;
                                }
                                else if (columns.at(i) == (~columns.at(j) & ~columns.at(k) & ~columns.at(l) & ~columns.at(m) & ~columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"and"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 1), uint32_t(l * 2 + 1), uint32_t(m * 2 + 1), uint32_t(i5 * 2 + 1)}}};
                                    break;
                                }
                            }
                            if (found)
                                break;
                        }
                        if (found)
                            break;
                    }
                    if (found)
                        break;
                }

                if (found)
                    break;
            }
            if (found)
                continue;

            /*-----or---------*/
            /*-----first input */
            for (auto j = uint32_t(columns.size()) - 1; j > i; --j)
            {
                if (columns.at(j).is_const())
                    continue;

                /*-----second input */
                for (auto k = j - 1; k > i; --k)
                {
                    if (columns.at(k).is_const())
                        continue;

                    if (columns.at(i) == (columns.at(j) | columns.at(k)))
                    {
                        found = true;
                        dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0)}}};
                        break;
                    }
                    else if (columns.at(i) == (~(columns.at(j) | columns.at(k))))
                    {
                        found = true;
                        dependencies[i] = std::vector{std::pair{std::string{"nor"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0)}}};
                        break;
                    }
                    else if (columns.at(i) == (~columns.at(j) | columns.at(k)))
                    {
                        found = true;
                        dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 0)}}};
                        break;
                    }
                    else if (columns.at(i) == (columns.at(j) | ~columns.at(k)))
                    {
                        found = true;
                        dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 1)}}};
                        break;
                    }
                    else if (columns.at(i) == (~columns.at(j) | ~columns.at(k)))
                    {
                        found = true;
                        dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 1)}}};
                        break;
                    }

                    /*------3rd input */
                    for (auto l = j - 2; l > i; --l)
                    {
                        if (columns.at(l).is_const())
                            continue;

                        if (columns.at(i) == (columns.at(j) | columns.at(k) | columns.at(l)))
                        {
                            found = true;
                            dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0)}}};
                            break;
                        }
                        else if (columns.at(i) == (~(columns.at(j) | columns.at(k) | columns.at(l))))
                        {
                            found = true;
                            dependencies[i] = std::vector{std::pair{std::string{"nor"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0)}}};
                            break;
                        }
                        else if (columns.at(i) == (~columns.at(j) | columns.at(k) | columns.at(l)))
                        {
                            found = true;
                            dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0)}}};
                            break;
                        }
                        else if (columns.at(i) == (columns.at(j) | ~columns.at(k) | columns.at(l)))
                        {
                            found = true;
                            dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 1), uint32_t(l * 2 + 0)}}};
                            break;
                        }
                        else if (columns.at(i) == (~columns.at(j) | ~columns.at(k) | columns.at(l)))
                        {
                            found = true;
                            dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 1), uint32_t(l * 2 + 0)}}};
                            break;
                        }
                        else if (columns.at(i) == (columns.at(j) | columns.at(k) | ~columns.at(l)))
                        {
                            found = true;
                            dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0), uint32_t(l * 2 + 1)}}};
                            break;
                        }
                        else if (columns.at(i) == (~columns.at(j) | columns.at(k) | ~columns.at(l)))
                        {
                            found = true;
                            dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 0), uint32_t(l * 2 + 1)}}};
                            break;
                        }
                        else if (columns.at(i) == (columns.at(j) | ~columns.at(k) | ~columns.at(l)))
                        {
                            found = true;
                            dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 1), uint32_t(l * 2 + 1)}}};
                            break;
                        }
                        else if (columns.at(i) == (~columns.at(j) | ~columns.at(k) | ~columns.at(l)))
                        {
                            found = true;
                            dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 1), uint32_t(l * 2 + 1)}}};
                            break;
                        }

                        /*---- 4th input */
                        for (auto m = j - 3; m > i; --m)
                        {
                            if (columns.at(m).is_const())
                                continue;

                            if (columns.at(i) == (columns.at(j) | columns.at(k) | columns.at(l) | columns.at(m)))
                            {
                                found = true;
                                dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0), uint32_t(m * 2 + 0)}}};
                                break;
                            }
                            else if (columns.at(i) == (~(columns.at(j) | columns.at(k) | columns.at(l) | columns.at(m))))
                            {
                                found = true;
                                dependencies[i] = std::vector{std::pair{std::string{"nor"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0), uint32_t(m * 2 + 0)}}};
                                break;
                            }

                            else if (columns.at(i) == (~columns.at(j) | columns.at(k) | columns.at(l) | columns.at(m)))
                            {
                                found = true;
                                dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0), uint32_t(m * 2 + 0)}}};
                                break;
                            }
                            else if (columns.at(i) == (columns.at(j) | ~columns.at(k) | columns.at(l) | columns.at(m)))
                            {
                                found = true;
                                dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 1), uint32_t(l * 2 + 0), uint32_t(m * 2 + 0)}}};
                                break;
                            }
                            else if (columns.at(i) == (~columns.at(j) | ~columns.at(k) | columns.at(l) | columns.at(m)))
                            {
                                found = true;
                                dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 1), uint32_t(l * 2 + 0), uint32_t(m * 2 + 0)}}};
                                break;
                            }
                            else if (columns.at(i) == (columns.at(j) | columns.at(k) | ~columns.at(l) | columns.at(m)))
                            {
                                found = true;
                                dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0), uint32_t(l * 2 + 1), uint32_t(m * 2 + 0)}}};
                                break;
                            }
                            else if (columns.at(i) == (~columns.at(j) | columns.at(k) | ~columns.at(l) | columns.at(m)))
                            {
                                found = true;
                                dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 0), uint32_t(l * 2 + 1), uint32_t(m * 2 + 0)}}};
                                break;
                            }
                            else if (columns.at(i) == (columns.at(j) | ~columns.at(k) | ~columns.at(l) | columns.at(m)))
                            {
                                found = true;
                                dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 1), uint32_t(l * 2 + 1), uint32_t(m * 2 + 0)}}};
                                break;
                            }
                            else if (columns.at(i) == (~columns.at(j) | ~columns.at(k) | ~columns.at(l) | columns.at(m)))
                            {
                                found = true;
                                dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 1), uint32_t(l * 2 + 1), uint32_t(m * 2 + 0)}}};
                                break;
                            }
                            else if (columns.at(i) == (columns.at(j) | columns.at(k) | columns.at(l) | ~columns.at(m)))
                            {
                                found = true;
                                dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0), uint32_t(m * 2 + 1)}}};
                                break;
                            }
                            else if (columns.at(i) == (~columns.at(j) | columns.at(k) | columns.at(l) | ~columns.at(m)))
                            {
                                found = true;
                                dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0), uint32_t(m * 2 + 1)}}};
                                break;
                            }
                            else if (columns.at(i) == (columns.at(j) | ~columns.at(k) | columns.at(l) | ~columns.at(m)))
                            {
                                found = true;
                                dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 1), uint32_t(l * 2 + 0), uint32_t(m * 2 + 1)}}};
                                break;
                            }
                            else if (columns.at(i) == (~columns.at(j) | ~columns.at(k) | columns.at(l) | ~columns.at(m)))
                            {
                                found = true;
                                dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 1), uint32_t(l * 2 + 0), uint32_t(m * 2 + 1)}}};
                                break;
                            }
                            else if (columns.at(i) == (columns.at(j) | columns.at(k) | ~columns.at(l) | ~columns.at(m)))
                            {
                                found = true;
                                dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0), uint32_t(l * 2 + 1), uint32_t(m * 2 + 1)}}};
                                break;
                            }
                            else if (columns.at(i) == (~columns.at(j) | columns.at(k) | ~columns.at(l) | ~columns.at(m)))
                            {
                                found = true;
                                dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 0), uint32_t(l * 2 + 1), uint32_t(m * 2 + 1)}}};
                                break;
                            }
                            else if (columns.at(i) == (columns.at(j) | ~columns.at(k) | ~columns.at(l) | ~columns.at(m)))
                            {
                                found = true;
                                dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 1), uint32_t(l * 2 + 1), uint32_t(m * 2 + 1)}}};
                                break;
                            }
                            else if (columns.at(i) == (~columns.at(j) | ~columns.at(k) | ~columns.at(l) | ~columns.at(m)))
                            {
                                found = true;
                                dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 1), uint32_t(l * 2 + 1), uint32_t(m * 2 + 1)}}};
                                break;
                            }

                            /*---- 5th input */
                            for (auto i5 = j - 4; i5 > i; --i5)
                            {
                                if (columns.at(i5).is_const())
                                    continue;

                                if (columns.at(i) == (columns.at(j) | columns.at(k) | columns.at(l) | columns.at(m) | columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0), uint32_t(m * 2 + 0), uint32_t(i5 * 2 + 0)}}};
                                    break;
                                }
                                else if (columns.at(i) == (~(columns.at(j) | columns.at(k) | columns.at(l) | columns.at(m) | columns.at(i5))))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"nor"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0), uint32_t(m * 2 + 0), uint32_t(i5 * 2 + 0)}}};
                                    break;
                                }
                                else if (columns.at(i) == (~columns.at(j) | columns.at(k) | columns.at(l) | columns.at(m) | columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0), uint32_t(m * 2 + 0), uint32_t(i5 * 2 + 0)}}};
                                    break;
                                }
                                else if (columns.at(i) == (columns.at(j) | ~columns.at(k) | columns.at(l) | columns.at(m) | columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 1), uint32_t(l * 2 + 0), uint32_t(m * 2 + 0), uint32_t(i5 * 2 + 0)}}};
                                    break;
                                }
                                else if (columns.at(i) == (~columns.at(j) | ~columns.at(k) | columns.at(l) | columns.at(m) | columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 1), uint32_t(l * 2 + 0), uint32_t(m * 2 + 0), uint32_t(i5 * 2 + 0)}}};
                                    break;
                                }
                                else if (columns.at(i) == (columns.at(j) | columns.at(k) | ~columns.at(l) | columns.at(m) | columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0), uint32_t(l * 2 + 1), uint32_t(m * 2 + 0), uint32_t(i5 * 2 + 0)}}};
                                    break;
                                }
                                else if (columns.at(i) == (~columns.at(j) | columns.at(k) | ~columns.at(l) | columns.at(m) | columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 0), uint32_t(l * 2 + 1), uint32_t(m * 2 + 0), uint32_t(i5 * 2 + 0)}}};
                                    break;
                                }
                                else if (columns.at(i) == (columns.at(j) | ~columns.at(k) | ~columns.at(l) | columns.at(m) | columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 1), uint32_t(l * 2 + 1), uint32_t(m * 2 + 0), uint32_t(i5 * 2 + 0)}}};
                                    break;
                                }
                                else if (columns.at(i) == (~columns.at(j) | ~columns.at(k) | ~columns.at(l) | columns.at(m) | columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 1), uint32_t(l * 2 + 1), uint32_t(m * 2 + 0), uint32_t(i5 * 2 + 0)}}};
                                    break;
                                }
                                else if (columns.at(i) == (columns.at(j) | columns.at(k) | columns.at(l) | ~columns.at(m) | columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0), uint32_t(m * 2 + 1), uint32_t(i5 * 2 + 0)}}};
                                    break;
                                }
                                else if (columns.at(i) == (~columns.at(j) | columns.at(k) | columns.at(l) | ~columns.at(m) | columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0), uint32_t(m * 2 + 1), uint32_t(i5 * 2 + 0)}}};
                                    break;
                                }
                                else if (columns.at(i) == (columns.at(j) | ~columns.at(k) | columns.at(l) | ~columns.at(m) | columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 1), uint32_t(l * 2 + 0), uint32_t(m * 2 + 1), uint32_t(i5 * 2 + 0)}}};
                                    break;
                                }
                                else if (columns.at(i) == (~columns.at(j) | ~columns.at(k) | columns.at(l) | ~columns.at(m) | columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 1), uint32_t(l * 2 + 0), uint32_t(m * 2 + 1), uint32_t(i5 * 2 + 0)}}};
                                    break;
                                }
                                else if (columns.at(i) == (columns.at(j) | columns.at(k) | ~columns.at(l) | ~columns.at(m) | columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0), uint32_t(l * 2 + 1), uint32_t(m * 2 + 1), uint32_t(i5 * 2 + 0)}}};
                                    break;
                                }
                                else if (columns.at(i) == (~columns.at(j) | columns.at(k) | ~columns.at(l) | ~columns.at(m) | columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 0), uint32_t(l * 2 + 1), uint32_t(m * 2 + 1), uint32_t(i5 * 2 + 0)}}};
                                    break;
                                }
                                else if (columns.at(i) == (columns.at(j) | ~columns.at(k) | ~columns.at(l) | ~columns.at(m) | columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 1), uint32_t(l * 2 + 1), uint32_t(m * 2 + 1), uint32_t(i5 * 2 + 0)}}};
                                    break;
                                }
                                else if (columns.at(i) == (~columns.at(j) | ~columns.at(k) | ~columns.at(l) | ~columns.at(m) | columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 1), uint32_t(l * 2 + 1), uint32_t(m * 2 + 1), uint32_t(i5 * 2 + 0)}}};
                                    break;
                                }
                                else if (columns.at(i) == (columns.at(j) | columns.at(k) | columns.at(l) | columns.at(m) | ~columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0), uint32_t(m * 2 + 0), uint32_t(i5 * 2 + 1)}}};
                                    break;
                                }

                                else if (columns.at(i) == (~columns.at(j) | columns.at(k) | columns.at(l) | columns.at(m) | ~columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0), uint32_t(m * 2 + 0), uint32_t(i5 * 2 + 1)}}};
                                    break;
                                }
                                else if (columns.at(i) == (columns.at(j) | ~columns.at(k) | columns.at(l) | columns.at(m) | ~columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 1), uint32_t(l * 2 + 0), uint32_t(m * 2 + 0), uint32_t(i5 * 2 + 1)}}};
                                    break;
                                }
                                else if (columns.at(i) == (~columns.at(j) | ~columns.at(k) | columns.at(l) | columns.at(m) | ~columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 1), uint32_t(l * 2 + 0), uint32_t(m * 2 + 0), uint32_t(i5 * 2 + 1)}}};
                                    break;
                                }
                                else if (columns.at(i) == (columns.at(j) | columns.at(k) | ~columns.at(l) | columns.at(m) | ~columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0), uint32_t(l * 2 + 1), uint32_t(m * 2 + 0), uint32_t(i5 * 2 + 1)}}};
                                    break;
                                }
                                else if (columns.at(i) == (~columns.at(j) | columns.at(k) | ~columns.at(l) | columns.at(m) | ~columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 0), uint32_t(l * 2 + 1), uint32_t(m * 2 + 0), uint32_t(i5 * 2 + 1)}}};
                                    break;
                                }
                                else if (columns.at(i) == (columns.at(j) | ~columns.at(k) | ~columns.at(l) | columns.at(m) | ~columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 1), uint32_t(l * 2 + 1), uint32_t(m * 2 + 0), uint32_t(i5 * 2 + 1)}}};
                                    break;
                                }
                                else if (columns.at(i) == (~columns.at(j) | ~columns.at(k) | ~columns.at(l) | columns.at(m) | ~columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 1), uint32_t(l * 2 + 1), uint32_t(m * 2 + 0), uint32_t(i5 * 2 + 1)}}};
                                    break;
                                }
                                else if (columns.at(i) == (columns.at(j) | columns.at(k) | columns.at(l) | ~columns.at(m) | ~columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0), uint32_t(m * 2 + 1), uint32_t(i5 * 2 + 1)}}};
                                    break;
                                }
                                else if (columns.at(i) == (~columns.at(j) | columns.at(k) | columns.at(l) | ~columns.at(m) | ~columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 0), uint32_t(l * 2 + 0), uint32_t(m * 2 + 1), uint32_t(i5 * 2 + 1)}}};
                                    break;
                                }
                                else if (columns.at(i) == (columns.at(j) | ~columns.at(k) | columns.at(l) | ~columns.at(m) | ~columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 1), uint32_t(l * 2 + 0), uint32_t(m * 2 + 1), uint32_t(i5 * 2 + 1)}}};
                                    break;
                                }
                                else if (columns.at(i) == (~columns.at(j) | ~columns.at(k) | columns.at(l) | ~columns.at(m) | ~columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 1), uint32_t(l * 2 + 0), uint32_t(m * 2 + 1), uint32_t(i5 * 2 + 1)}}};
                                    break;
                                }
                                else if (columns.at(i) == (columns.at(j) | columns.at(k) | ~columns.at(l) | ~columns.at(m) | ~columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 0), uint32_t(l * 2 + 1), uint32_t(m * 2 + 1), uint32_t(i5 * 2 + 1)}}};
                                    break;
                                }
                                else if (columns.at(i) == (~columns.at(j) | columns.at(k) | ~columns.at(l) | ~columns.at(m) | ~columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 0), uint32_t(l * 2 + 1), uint32_t(m * 2 + 1), uint32_t(i5 * 2 + 1)}}};
                                    break;
                                }
                                else if (columns.at(i) == (columns.at(j) | ~columns.at(k) | ~columns.at(l) | ~columns.at(m) | ~columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 0), uint32_t(k * 2 + 1), uint32_t(l * 2 + 1), uint32_t(m * 2 + 1), uint32_t(i5 * 2 + 1)}}};
                                    break;
                                }
                                else if (columns.at(i) == (~columns.at(j) | ~columns.at(k) | ~columns.at(l) | ~columns.at(m) | ~columns.at(i5)))
                                {
                                    found = true;
                                    dependencies[i] = std::vector{std::pair{std::string{"or"}, std::vector<uint32_t>{uint32_t(j * 2 + 1), uint32_t(k * 2 + 1), uint32_t(l * 2 + 1), uint32_t(m * 2 + 1), uint32_t(i5 * 2 + 1)}}};
                                    break;
                                }
                            }
                            if (found)
                                break;
                        }
                        if (found)
                            break;
                    }
                    if (found)
                        break;
                }

                if (found)
                    break;
            }
            if (found)
                continue;
            /// remove and_xor, and_xnor, or_xor, or_xnor
        }

        if (dependencies.size() == 0u)
        {
            ++stats.no_dependencies_computed;
        }
        else if (dependencies.size() > 0u)
        {
            ++stats.has_dependencies;
        }

        return dependencies;
    }


}; /// class ResubSynthesisDeps end

} /// namespace angel end