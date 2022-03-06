from ahocorasick import Automaton


def test_issue_53():
    auto = Automaton()
    auto.add_word('wounded', 'wounded')

    auto.make_automaton()

    results = list(auto.iter('Winning \U0001F629 so gutted, can\'t do anything for 4 weeks... Myth. #wounded'))
    expected = [(67, 'wounded')]
    assert results == expected

    expected = [(65, 'wounded')]
    results = list(auto.iter('Winning so gutted, can\'t do anything for 4 weeks... Myth. #wounded'))
    assert results == expected
