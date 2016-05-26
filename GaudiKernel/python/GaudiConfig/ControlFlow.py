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
        return (str(self) == str(other))


class Algorithm(ControlFlowNode):
    def __init__(self, name):
        self.name = name

    def __str__(self):
        return self.name


class OrderedNode(ControlFlowNode):
    def __init__(self, lhs, rhs):
        self.lhs = lhs
        self.rhs = rhs

    def __str__(self):
        return "%s >> %s" % (self.lhs, self.rhs)

    def _visitSubNodes(self, visitor):
        self.lhs.visitNode(visitor)
        self.rhs.visitNode(visitor)


class AndNode(ControlFlowNode):
    def __init__(self, lhs, rhs):
        self.lhs = lhs
        self.rhs = rhs

    def __str__(self):
        return "%s & %s" % (self.lhs, self.rhs)

    def _visitSubNodes(self, visitor):
        self.lhs.visitNode(visitor)
        self.rhs.visitNode(visitor)


class OrNode(ControlFlowNode):
    def __init__(self, lhs, rhs):
        self.lhs = lhs
        self.rhs = rhs

    def __str__(self):
        return "%s | %s" % (self.lhs, self.rhs)

    def _visitSubNodes(self, visitor):
        self.lhs.visitNode(visitor)
        self.rhs.visitNode(visitor)


class InvertNode(ControlFlowNode):
    def __init__(self, item):
        self.item = item

    def __str__(self):
        return "~%s" % self.item

    def _visitSubNodes(self, visitor):
        self.item.visitNode(visitor)


class IgnoreNode(ControlFlowNode):
    def __init__(self, item):
        self.item = item

    def __str__(self):
        return "ignore(%s)" % self.item

    def _visitSubNodes(self, visitor):
        self.item.visitNode(visitor)


def ignore(item):
    return IgnoreNode(item)


class ParallelExecutionNode(ControlFlowNode):
    def __init__(self, item):
        self.item = item

    def __str__(self):
        return "par(%s)" % self.item

    def _visitSubNodes(self, visitor):
        self.item.visitNode(visitor)


def par(item):
    return ParallelExecutionNode(item)


class seq(ControlFlowNode):
    def __init__(self, item):
        self.item = item

    def __str__(self):
        return "seq(%s)" % self.item

    def _visitSubNodes(self, visitor):
        self.item.visitNode(visitor)


class line(object):
    def __init__(self, name, item):
        self.name = name
        self.item = item

    def __str__(self):
        return "line('%s', %s)" % (self.name, self.item)

    def _visitSubNodes(self, visitor):
        self.item.visitNode(visitor)


class Visitor(object):
    def __init__(self):
        self.depths = 0

    def enter(self, visitee):
        self.depths += 1
        print "%sEntering %s" % (self.depths*" ", type(visitee))
        if isinstance(visitee, Algorithm):
            print "%s Algorithm name: %s" % (" "*self.depths, visitee)

    def leave(self, visitee):
        print "%sLeaving %s" % (self.depths * " ", type(visitee))
        self.depths -= 1


def test():
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
