#include <iostream>
#include <memory>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <map>
#include <queue>

#include "utils/cache/lru.h"
#include "database/init.h"
#include "database/dao/pcu_relay_cnt/pcu_relay_cnt.h"
#include "fcgi/fcgi.h"
#include "relay/relay.h"
#include "utils/priority_queue/priority_queue.h"
#include "can/can.h"
#include "peventloop/monotonic.h"
#include "utils/timer.h"

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

        int jump(vector<int>& nums) {
            auto n = nums.size();
            int jumps = 0, farthest = 0, current_end = 0;

            for (auto i = 0; i < n - 1; i++)
            {
                farthest = max(farthest, i + nums[i]);
                if (i == current_end)
                {
                    jumps++;
                    d_log("jumps[%d], farthest[%d] n-1[%d]", jumps, farthest, n - 1);
                    current_end = farthest;
                    if (current_end >= n - 1)
                    {
                        break;
                    }
                }

            }
            return jumps;
        }
};


static void my_callback(void *userdata)
{
    d_log("Timer1 fired! Data: %s", (char*)userdata);
}

static void my_callback1(void *userdata)
{
    d_log("Timer2 fired! Data: %s", (char*)userdata);
}


static void test_timer(void)
{
    int quit = 0;
    p_timer_init(&quit);
    char *data = strdup("Hello Timer");
    Timer_handle_t t1 = p_timer_add(1000, 3, &my_callback, data);
    Timer_handle_t t2 = p_timer_add(1000, 3, &my_callback1, data);
    // // 主线程做其他工作
    while (1)
    {
        sleep(5);
    }
    quit = 1;
    p_timer_del(&t1);
    p_timer_del(&t2);
}

int main(void)
{
    test_timer();
    return 0;
}
