#include <iostream>
#include <memory>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <map>

#include "utils/cache/lru.h"
#include "database/init.h"
#include "database/dao/pcu_relay_cnt/pcu_relay_cnt.h"
#include "fcgi/fcgi.h"
#include "relay/relay.h"

#include "mode/singleton.h"
#include "mode/composite.h"
#include "mode/template.h"

using namespace std;


class Solution
{
    public:
        void merge(vector<int>& nums1, int m, vector<int>& nums2, int n)
        {
            int i = m - 1;
            int j = n - 1;
            int k = m + n - 1;
            while (i >= 0 && j>=0)
            {
                if (nums1[i] > nums2[j])
                {
                    nums1[k--] = nums1[i--];
                }
                else
                {
                    nums1[k--] = nums2[j--];
                }
            }
            while (j >= 0) {
                nums1[k--] = nums2[j--];
            }
        }
        int removeElement(vector<int>& nums, int val) {
            int size = nums.size();
            vector<int> nums2;
            int num = 0;
            for (int i = 0; i < size; i++)
            {
                if (nums[i] != val)
                {
                    nums2.push_back(nums[i]);
                    num++;
                }
            }
            nums.resize(num);
            int k = 0;
            for (int j : nums2)
            {
                nums[k++] = j;
                d_log("j:%d", j);
            }
            return num;
        }
        int removeDuplicates(vector<int>& nums) {
            if (nums.empty()) return 0;
            int k = 1;
            int count = 1;
            for (int i = 1; i < nums.size(); i++)
            {
                if (nums[i] == nums[k-1])
                {
                    count++;
                }
                else
                {
                    count = 1;
                }

                if (count <=2)
                {
                    nums[k++] = nums[i];
                }
            }
            nums.resize(k);
            return k;
        }

        int majorityElement(vector<int>& nums) {
            unordered_map<int, int>counts;
            int majority = 0, cnt = 0;
            for (int i : nums)
            {
                counts[i]++;
                if (counts[i] > cnt)
                {
                    majority = i;
                    cnt = counts[i];
                }
            }
            return majority;
        }

        void rotate(vector<int>& nums, int k) {
            reverse(nums.begin(), nums.end());
            reverse(nums.begin(), nums.begin() + k - 1);
            reverse(nums.begin() + k, nums.end());
        }

        int maxProfit(vector<int>& prices) {
            int ans = 0;
            for (uint8_t i = 0; i < prices.size() - 1; i++)
            {
                if (prices[i] < prices[i + 1])
                {
                    ans += (prices[i + 1] - prices[i]);
                }
            }
            return ans;
        }

        bool canJump(vector<int>& nums) {
            int k = 0;
            for(int i = 0; i < nums.size(); i++) {
                if(k < i) return false;
                k = max(k, i + nums[i]);
            }
            return true;
        }

};


int main(void)
{

    // log_init();
    // vector<int>nums1 = {2,3,1,1,4};
    // Solution solution;
    // int ret =solution.canJump(nums1);
    // for (int i:nums1)
    // {
    //     d_log("ret[%d] %d", ret, i);
    // }
    // dao_t *dao = dao_init();
    // pcu_relay_cnt_t *relay_cnt = get_relay_cnt();
    // for (uint8_t i = 0; i < 6; i++)
    // {
    //     pcu_relay_cnt_dao_create(&dao->pcu_relay_cnt_dao, &relay_cnt[i]);
    // }

    // pcu_do_branch_relay_e relay_id = DO_DC_INPUT2_PRE;
    // pcu_relay_cnt_t res;
    // pcu_relay_cnt_dao_get_by_relay_id(&dao->pcu_relay_cnt_dao, relay_id, &res);
    // d_log("DO_DC_INPUT2_PRE cnt:%d", res.close_cnt);

    // pcu_relay_cnt_dao_delete_by_relay_id(&dao->pcu_relay_cnt_dao, DO_DC_INPUT1_POS);
    // pcu_relay_cnt_t p = {7, DO_DC_INPUT1_NEG, 212};
    // pcu_relay_cnt_dao_update_by_relay_id(&dao->pcu_relay_cnt_dao, DO_DC_INPUT1_NEG, &p);

    // fcgi_main(dao);

    // dao_clear();

    return 0;
}

