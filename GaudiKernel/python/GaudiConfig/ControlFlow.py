class ControlFlowNode(object):
    def __init__(self):
        pass

    def __and__(self, rhs):
        return AndNode(self, rhs)

    def __or__(self, rhs):
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
        return (repr(self) == repr(other))


class ControlFlowLeaf(ControlFlowNode):
    pass


class OrderedNode(ControlFlowNode):
    def __init__(self, lhs, rhs):
        self.lhs = lhs
        self.rhs = rhs

    def __repr__(self):
        return "%r >> %r" % (self.lhs, self.rhs)

    def _visitSubNodes(self, visitor):
        self.lhs.visitNode(visitor)
        self.rhs.visitNode(visitor)


class AndNode(ControlFlowNode):
    def __init__(self, lhs, rhs):
        self.lhs = lhs
        self.rhs = rhs

    def __repr__(self):
        return "(%r & %r)" % (self.lhs, self.rhs)

    def _visitSubNodes(self, visitor):
        self.lhs.visitNode(visitor)
        self.rhs.visitNode(visitor)


class OrNode(ControlFlowNode):
    def __init__(self, lhs, rhs):
        self.lhs = lhs
        self.rhs = rhs

    def __repr__(self):
        return "(%r | %r)" % (self.lhs, self.rhs)

    def _visitSubNodes(self, visitor):
        self.lhs.visitNode(visitor)
        self.rhs.visitNode(visitor)


class InvertNode(ControlFlowNode):
    def __init__(self, item):
        self.item = item

    def __repr__(self):
        return "~%r" % self.item

    def _visitSubNodes(self, visitor):
        self.item.visitNode(visitor)


class IgnoreNode(ControlFlowNode):
    def __init__(self, item):
        self.item = item

    def __repr__(self):
        return "ignore(%r)" % self.item

    def _visitSubNodes(self, visitor):
        self.item.visitNode(visitor)


def ignore(item):
    return IgnoreNode(item)


class ParallelExecutionNode(ControlFlowNode):
    def __init__(self, item):
        self.item = item

    def __repr__(self):
        return "par(%r)" % self.item

    def _visitSubNodes(self, visitor):
        self.item.visitNode(visitor)


def par(item):
    return ParallelExecutionNode(item)


class seq(ControlFlowNode):
    def __init__(self, item):
        self.item = item

    def __repr__(self):
        return "seq(%r)" % self.item

    def _visitSubNodes(self, visitor):
        self.item.visitNode(visitor)


class line(object):
    def __init__(self, name, item):
        self.name = name
        self.item = item

    def __repr__(self):
        return "line(%r, %r)" % (self.name, self.item)

    def _visitSubNodes(self, visitor):
        self.item.visitNode(visitor)


class Visitor(object):
    def __init__(self):
        self.depths = 0

    def enter(self, visitee):
        self.depths += 1
        print "%sEntering %s" % (self.depths*" ", type(visitee))
        if isinstance(visitee, ControlFlowLeaf):
            print "%s Algorithm name: %s" % (" "*self.depths, visitee)

    def leave(self, visitee):
        print "%sLeaving %s" % (self.depths * " ", type(visitee))
        self.depths -= 1


class CreateSequencesVisitor(object):
    def __init__(self):
        self.stack = []

    @property
    def sequence(self):
        return self.stack[-1]

    def enter(self, visitee):
        pass

    def leave(self, visitee):
        if isinstance(visitee, ControlFlowLeaf):
            self.stack.append(visitee)
        elif isinstance(visitee, (OrNode, AndNode, OrderedNode)):
            b = self.stack.pop()
            a = self.stack.pop()
            self.stack.append([a, b])


class _TestAlgorithm(ControlFlowLeaf):
    def __init__(self, name):
        self.name = name
    def __repr__(self):
        return self.name


def test():
    Algorithm = _TestAlgorithm

    a = Algorithm("a")
    b = Algorithm("b")
    c = Algorithm("c")
    d = Algorithm("d")
    e = Algorithm("e")
    sequence = seq(b >> a)
    expression = sequence | ~c & par(d & e)
    a = (expression == expression)
    aLine = line("MyTriggerPath", expression)
    visitor = Visitor()
    print "\nPrinting trigger line:"
    print aLine
    print "\nPrinting expression:"
    print expression
    print "\nTraversing through expression:\n"
    expression.visitNode(visitor)

def test_sequences():
    Algorithm = _TestAlgorithm

    a = Algorithm("a")
    b = Algorithm("b")
    c = Algorithm("c")
    d = Algorithm("d")
    e = Algorithm("e")
    sequence = seq(b >> a)
    expression = sequence | ~c & par(d & e)
    visitor = CreateSequencesVisitor()
    expression.visitNode(visitor)
    print visitor.sequence
    assert visitor.sequence == [[b, a], [c, [d, e]]]
