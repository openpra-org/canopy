/// @file
/// Fault Tree and Component containers.

#pragma once

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "mef/openpsa/ccf_group.h"
#include "mef/openpsa/element.h"
#include "mef/openpsa/event.h"
#include "mef/openpsa/parameter.h"

namespace mef::openpsa {

/// Component is for logical grouping of events, gates, and other components.
class Component
   : public Element,
     public Role,
     public Composite<Container<Component, Component, true, false>,
                      Container<Component, BasicEvent, false, false>,
                      Container<Component, HouseEvent, false, false>,
                      Container<Component, Gate, false, false>,
                      Container<Component, Parameter, false, false>,
                      Container<Component, CcfGroup, false, false>> {
 public:
   /// Type identifier string for error messages.
   static constexpr const char* kTypeString = "component/fault tree";

   /// Constructs a component assuming
   /// that it exists within some fault tree.
   /// The public or private role of a component is not
   /// for the components itself,
   /// but for the events and parameters of the component.
   /// Component name is not meant to be public;
   /// however, it must be unique within the parent fault tree or component.
   ///
   /// @param[in] name  The name identifier for the component.
   /// @param[in] base_path  The series of containers to get this container.
   /// @param[in] role  The default role for container members.
   ///
   /// @throws LogicError  The name is empty.
   /// @throws ValidityError  The name or reference paths are malformed.
   explicit Component(std::string name, std::string base_path = "",
                      RoleSpecifier role = RoleSpecifier::kPublic) : Element(std::move(name)), Role(role, std::move(base_path)) {}

   virtual ~Component() = default;

   /// @returns The table ranges of component elements of specific kind
   ///          with element original names as keys.
   /// @{
   auto gates() const { return table<Gate>(); }
   auto basic_events() const { return table<BasicEvent>(); }
   auto house_events() const { return table<HouseEvent>(); }
   auto parameters() const { return table<Parameter>(); }
   auto ccf_groups() const { return table<CcfGroup>(); }
   auto components() const { return table<Component>(); }
   /// @}

   using Composite::Add;
   using Composite::Remove;

   /// Adds MEF constructs into this component container.
   ///
   /// @param[in] element  The element to be added to the container.
   ///
   /// @throws DuplicateElementError  The element is already in this container.
   ///
   /// @{
   void Add(Gate* element) { AddEvent(element); }

   void Add(BasicEvent* element) { AddEvent(element); }

   void Add(HouseEvent* element) { AddEvent(element); }

   void Add(CcfGroup* ccf_group) {
       if (ccf_groups().count(ccf_group->name())) {
           throw(DuplicateElementError("CCF group: " + ccf_group->name()));
       }
       for (BasicEvent* member : ccf_group->members())
           CheckDuplicateEvent(*member);

       for (auto* member : ccf_group->members())
           Composite::Add(member);

       Composite::Add(ccf_group);
   }
   /// @}

 protected:
   /// Recursively traverses components
   /// to gather gates relevant to the whole component.
   ///
   /// @param[out] gates  Gates belonging to this component
   ///                    and its subcomponents.
   void GatherGates(std::unordered_set<Gate*>* gates) {
       gates->insert(data<Gate>().begin(), data<Gate>().end());
       for (Component& component : table<Component>())
           component.GatherGates(gates);
   }

 private:
   /// @copydoc Component::Add(BasicEvent*)
   template <class T>
   void AddEvent(T* element) {
       CheckDuplicateEvent(*element);
       Composite::Add(element);
   }

   /// Checks if an event with the same id is already in the component.
   ///
   /// @param[in] event  The event to be tested for duplicate before insertion.
   ///
   /// @throws DuplicateElementError  The element is already in the model.
   void CheckDuplicateEvent(const Event& event) {
       const std::string& name = event.name();
       if (gates().count(name) || basic_events().count(name) ||
           house_events().count(name)) {
           throw(DuplicateElementError("event: "+name +", element: "+kTypeString));
       }
   }
};

/// Fault tree representation as a container of
/// gates, basic and house events, and other information.
/// Additional functionality of a fault tree includes
/// detection of top events.
class FaultTree : public Component {
 public:
   /// Type identifier string for error messages.
   static constexpr const char* kTypeString = "fault tree";

   /// The main constructor of the Fault Tree.
   /// Fault trees are assumed to be public and belong to the root model.
   ///
   /// @param[in] name  The name identifier of this fault tree.
   explicit FaultTree(const std::string& name) : Component(name) {}

   /// @returns The collected top events of this fault tree.
   const std::vector<const Gate*>& top_events() const { return top_events_; }

   /// Collects top event gates in this fault tree with components.
   /// This function is essential to guess the analysis targets
   /// if the user does not supply any.
   /// If the structure of the fault tree changes,
   /// this function must be called again to update the top events.
   ///
   /// @pre Gate marks are clear.
   void CollectTopEvents() {
       top_events_.clear();
       std::unordered_set<Gate*> gates;
       Component::GatherGates(&gates);
       // Detects top events.
       for (Gate* gate : gates)
           MarkNonTopGates(gate, gates);

       for (Gate* gate : gates) {
           if (gate->mark()) {  // Not a top event.
               gate->mark(NodeMark::kClear);  // Cleaning up.
           } else {
               top_events_.push_back(gate);
           }
       }
   }

 private:
   /// Recursively marks descendant gates
   /// with an unspecified-but-non-clear mark.
   /// These gates belong to this fault tree only.
   ///
   /// @param[in,out] gate  The ancestor gate.
   /// @param[in] gates  Gates belonging to the whole fault tree with components.
   void MarkNonTopGates(Gate* gate, const std::unordered_set<Gate*>& gates) {
       if (gate->mark())
           return;
       for (const Formula::Arg& arg : gate->formula().args()) {
           if (Gate* const* arg_gate = std::get_if<Gate*>(&arg.event)) {
               if (gates.count(*arg_gate)) {
                   MarkNonTopGates(*arg_gate, gates);
                   (*arg_gate)->mark(NodeMark::kPermanent);  // Any non clear mark.
               }
           }
       }
   }

   std::vector<const Gate*> top_events_;  ///< Top events of this fault tree.
};

}  // namespace scram::mef
