#####################################################################################
# (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
"""
Classes for the implementation of the Control Flow Structure Syntax.

@see: https://github.com/lhcb/scheduling-event-model/tree/master/controlflow_syntax
"""


class ControlFlowNode:
    """
    Basic entry in the control flow graph.
    """

    def __and__(self, rhs):
        if rhs is CFTrue:
            return self
        elif rhs is CFFalse:
            return CFFalse
        return AndNode(self, rhs)

    def __or__(self, rhs):
        if rhs is CFFalse:
            return self
        elif rhs is CFTrue:
            return CFTrue
        return OrNode(self, rhs)

    def __invert__(self):
        return InvertNode(self)

    def __rshift__(self, rhs):
        return OrderedNode(self, rhs)

    def visitNode(self, visitor):
        visitor.enter(self)
        self._visitSubNodes(visitor)
        visitor.leave(self)

    def _visitSubNodes(self, visitor):
        pass

    def __eq__(self, other):
        return repr(self) == repr(other)

    def __hash__(self):
        """Return a unique identifier for this object.

        As we use the `repr` of this object to check for equality, we use it
        here to define uniqueness.
        """
        # The hash of the 1-tuple containing the repr of this object
        return hash((repr(self),))

    def getFullName(self):
        """
        Allow use of an expression as an algorihtm/sequence in a Gaudi job
        configuration.

        Convert the expression in nested sequencers and return the full name of
        the top one.
        """
        if not hasattr(self, "_fullname"):
            from GaudiKernel.Configurable import makeSequences

            self._fullname = makeSequences(self).getFullName()
        return self._fullname


class ControlFlowLeaf(ControlFlowNode):
    """
    Class used to identify a note without sub-nodes.
    """

    pass


class ControlFlowBool(ControlFlowLeaf):
    def __init__(self, value):
        self.value = value

    def __and__(self, rhs):
        return rhs if self.value else self

    def __or__(self, rhs):
        return self if self.value else rhs

    def __invert__(self):
        return CFFalse if self.value else CFTrue

    def __repr__(self):
        return "CFTrue" if self.value else "CFFalse"


CFTrue = ControlFlowBool(True)
CFFalse = ControlFlowBool(False)
del ControlFlowBool


class OrderedNode(ControlFlowNode):
    """
    Represent order of execution of nodes.
    """

    def __init__(self, lhs, rhs):
        self.lhs = lhs
        self.rhs = rhs

    def __repr__(self):
        return f"({self.lhs!r} >> {self.rhs!r})"

    def _visitSubNodes(self, visitor):
        self.lhs.visitNode(visitor)
        self.rhs.visitNode(visitor)


class AndNode(ControlFlowNode):
    """
    And operation between control flow nodes.
    """

    def __init__(self, lhs, rhs):
        self.lhs = lhs
        self.rhs = rhs

    def __repr__(self):
        return f"({self.lhs!r} & {self.rhs!r})"

    def _visitSubNodes(self, visitor):
        self.lhs.visitNode(visitor)
        self.rhs.visitNode(visitor)


class OrNode(ControlFlowNode):
    """
    Or operation between control flow nodes.
    """

    def __init__(self, lhs, rhs):
        self.lhs = lhs
        self.rhs = rhs

    def __repr__(self):
        return f"({self.lhs!r} | {self.rhs!r})"

    def _visitSubNodes(self, visitor):
        self.lhs.visitNode(visitor)
        self.rhs.visitNode(visitor)


class InvertNode(ControlFlowNode):
    """
    Invert logic (negation) of a control flow node.
    """

    def __init__(self, item):
        self.item = item

    def __repr__(self):
        return f"~{self.item!r}"

    def _visitSubNodes(self, visitor):
        self.item.visitNode(visitor)


class ignore(ControlFlowNode):
    """
    Treat a control flow node as always successful, equivalent to (a | ~ a).
    """

    def __init__(self, item):
        self.item = item

    def __repr__(self):
        return f"ignore({self.item!r})"

    def _visitSubNodes(self, visitor):
        self.item.visitNode(visitor)


class par(ControlFlowNode):
    def __init__(self, item):
        self.item = item

    def __repr__(self):
        return f"par({self.item!r})"

    def _visitSubNodes(self, visitor):
        self.item.visitNode(visitor)


class seq(ControlFlowNode):
    def __init__(self, item):
        self.item = item

    def __repr__(self):
        return f"seq({self.item!r})"

    def _visitSubNodes(self, visitor):
        self.item.visitNode(visitor)


class line:
    def __init__(self, name, item):
        self.name = name
        self.item = item

    def __repr__(self):
        return f"line({self.name!r}, {self.item!r})"

    def _visitSubNodes(self, visitor):
        self.item.visitNode(visitor)


class _TestVisitor:
    def __init__(self):
        self.depths = 0

    def enter(self, visitee):
        self.depths += 1
        print("{}Entering {}".format(self.depths * " ", type(visitee)))
        if isinstance(visitee, ControlFlowLeaf):
            print("{} Algorithm name: {}".format(" " * self.depths, visitee))

    def leave(self, visitee):
        print("{}Leaving {}".format(self.depths * " ", type(visitee)))
        self.depths -= 1


class _TestAlgorithm(ControlFlowLeaf):
    def __init__(self, name):
        self._name = name

    def __repr__(self):
        return self._name

    def name(self):
        return self._name


class DotVisitor:
    def __init__(self):
        self.nodes = []
        self.edges = []
        self.number = 0
        self.stack = []
        self.ids = {}

    def enter(self, visitee):
        if visitee not in self.ids:
            self.number += 1
            dot_id = self.ids[visitee] = f"T{self.number}"
        dot_id = self.ids[visitee]
        mother = None
        if self.is_needed(visitee):
            if isinstance(visitee, ControlFlowLeaf):
                entry = f'{dot_id} [label="{visitee.name()}", shape=box]'
            elif isinstance(visitee, OrNode):
                entry = f'{dot_id} [label="OR", shape=invhouse]'
            elif isinstance(visitee, AndNode):
                entry = f'{dot_id} [label="AND", shape=invhouse]'
            elif isinstance(visitee, OrderedNode):
                entry = f'{dot_id} [label=">>", shape=point]'
            elif isinstance(visitee, InvertNode):
                entry = f'{dot_id} [label="NOT", shape=circle, color=red]'
            elif isinstance(visitee, par):
                entry = f'{dot_id} [label="PAR", shape=circle]'
            elif isinstance(visitee, seq):
                entry = f'{dot_id} [label="SEQ", shape=circle]'
            else:
                entry = f'{dot_id} [label="{type(visitee)}", shape=circle]'
            self.nodes.append(entry)
            if len(self.stack) != 0:
                mother = self.collapse_identical_ancestors(type(self.stack[-1][0]))
                if not mother:
                    mother = self.stack[-1][1]
                edge = f"{dot_id}->{mother}"
                self.edges.append(edge)
        self.stack.append((visitee, dot_id))

    def leave(self, visitee):
        self.stack.pop()

    def collapse_identical_ancestors(self, thetype):
        """
        If AND nodes are inside AND nodes, the graph could be simplified
        to not contain those (same true for OR and ordered)
        """
        if len(self.stack) != 0:
            mother = self.stack[-1][1]
            for entry in self.stack[::-1]:
                if not isinstance(entry[0], thetype):
                    break
                mother = entry[1]
            return mother
        return None

    def is_needed(self, visitee):
        """
        Check whether this node is actually needed
        """
        if len(self.stack) != 0:
            return not isinstance(visitee, type(self.stack[-1][0]))
        return True

    def write(self, filename):
        output = """
digraph graphname {{
rankdir=LR

{}

{}

}}
""".format(
            "\n".join(self.nodes),
            "\n".join(self.edges),
        )

        with open(filename, "w") as outfile:
            outfile.write(output)


def test():
    Algorithm = _TestAlgorithm

    a = Algorithm("a")
    b = Algorithm("b")
    c = Algorithm("c")
    d = Algorithm("d")
    e = Algorithm("e")
    f = Algorithm("f")
    g = Algorithm("g")
    sequence = seq(b >> a >> f)
    expression = sequence | ~c & par(d & e & g)
    a = expression == expression
    aLine = line("MyTriggerPath", expression)
    visitor = _TestVisitor()
    visitor2 = DotVisitor()
    print("\nPrinting trigger line:")
    print(aLine)
    print("\nPrinting expression:")
    print(expression)
    print("\nTraversing through expression:\n")
    expression.visitNode(visitor)
    expression.visitNode(visitor2)
    visitor2.write("out.dot")
