//
// Created by Darwin Yuan on 2020/6/13.
//

#ifndef TRANS_DSL_2_CONCURRENTHELPER_H
#define TRANS_DSL_2_CONCURRENTHELPER_H

#include <trans-dsl/tsl_ns.h>
#include <trans-dsl/utils/SeqInt.h>
#include <trans-dsl/sched/action/SchedConcurrent.h>
#include <trans-dsl/sched/helper/IsSchedAction.h>

TSL_NS_BEGIN

struct SchedAction;

namespace details {
   template<SeqInt V_SEQ, typename ... T_ACTIONS>
   struct GenericConcurrent;

//   template<typename T_ACTION>
//   using IsEmptySchedAction =
//      std::enable_if_t<
//         std::is_base_of_v<SchedAction, T_ACTION> &&
//            sizeof(T_ACTION) == sizeof(SchedAction)>;

   template<SeqInt V_SEQ, typename T_HEAD, typename ... T_TAIL>
   struct GenericConcurrent<V_SEQ, T_HEAD, T_TAIL...> {
      using Action = T_HEAD;
      using Next =
      typename GenericConcurrent<
         V_SEQ + 1,
         T_TAIL...>::Inner;

      struct Inner : Next {
         auto get(SeqInt seq) -> SchedAction * {
            return seq == V_SEQ ? &action : Next::get(seq);
         }
      private:
         Action action;
      };
   };

   template<SeqInt V_SEQ>
   struct GenericConcurrent<V_SEQ> {
      struct Inner {
         auto get(SeqInt seq) -> SchedAction * {
            return nullptr;
         }
      };
   };

   template<typename ... T_ACTIONS>
   struct Concurrent__ {
      using Actions = typename GenericConcurrent<0, T_ACTIONS...>::Inner;
      enum {
         Num_Of_Actions = sizeof...(T_ACTIONS)
      };

      struct Inner : SchedConcurrent, private Actions {
         enum  {
            Max_Num_Of_Actions = SchedConcurrent::Max_Num_Of_Children
         };
      private:
         OVERRIDE(getNumOfActions() const -> SeqInt) {
            return Num_Of_Actions;
         }

         OVERRIDE(get(SeqInt index) -> SchedAction*) {
            return Actions::get(index);
         }
      };

      static_assert(Num_Of_Actions >= 2, "# of concurrent actions should be at least 2");
      static_assert(Num_Of_Actions <= Concurrent__<T_ACTIONS...>::Num_Of_Actions, "too much actions in __concurrent");
   };
}

#define __concurrent(...) TSL_NS::details::Concurrent__<__VA_ARGS__>::Inner

TSL_NS_END

#endif //TRANS_DSL_2_CONCURRENTHELPER_H
