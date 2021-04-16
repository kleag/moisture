from django.core.mail import EmailMessage
from django.contrib.auth.models import User
from rest_framework import mixins
from rest_framework import generics
from rest_framework import permissions
from rest_framework import renderers
from rest_framework import status
from rest_framework import viewsets
from rest_framework.decorators import action
from rest_framework.decorators import api_view
from rest_framework.response import Response
from rest_framework.reverse import reverse
from rest_framework.reverse import reverse_lazy

from plotly.offline import plot
from plotly.graph_objs import Scatter
from plotly.graph_objs import Bar
import plotly.graph_objs as go

from moistureduino.models import Entry
from moistureduino.serializers import EntrySerializer
from moistureduino.serializers import UserSerializer
from moistureduino.permissions import IsOwnerOrReadOnly

import moisture.settings as settings

class EntryViewSet(viewsets.ModelViewSet):
    """
    This viewset automatically provides `list`, `create`, `retrieve`,
    `update` and `destroy` actions.

    Additionally we also provide extra `highlight`, `table`, `plot` and
    `reset` actions.
    """
    queryset = Entry.objects.all()
    serializer_class = EntrySerializer
    permission_classes = [permissions.IsAuthenticatedOrReadOnly,
                          IsOwnerOrReadOnly]

    @action(detail=False,
            permission_classes=[permissions.IsAuthenticated])
    def reset(self, request, *args, **kwargs):
        entries = Entry.objects.all()
        for entry in entries:
            entry.delete()
        api_root = reverse_lazy('api-root', request=request)
        return Response(status=status.HTTP_204_NO_CONTENT, data=api_root)

    @action(detail=True, renderer_classes=[renderers.StaticHTMLRenderer])
    def highlight(self, request, *args, **kwargs):
        entry = self.get_object()
        return Response(entry.highlighted)

    @action(detail=False, renderer_classes=[renderers.StaticHTMLRenderer])
    def table(self, request, *args, **kwargs):
        entries = Entry.objects.all()
        rows = []
        for entry in entries:
            cells = []
            cells.append(f"<td>{entry.created}</td>")
            cells.append(f"<td>{entry.kind}</td>")
            cells.append(f"<td>{entry.value}</td>")
            scells = ''.join(cells)
            row = f"  <tr>{scells}</tr>"
            rows.append(row)
        srow = '\n'.join(rows)
        html = f"<html>\n<body>\n<table>\n{srow}\n</table>\n</body>\n</html>"
        return Response(html)

    @action(detail=False, renderer_classes=[renderers.TemplateHTMLRenderer])
    def plot(self, request, *args, **kwargs):

        x_labels = []
        x_data = []
        y_data = []
        xbar_data = []
        ybar_data = []
        entries = Entry.objects.all()
        for i, entry in enumerate(entries):
            x_labels.append(entry.created)
            if entry.kind == "moisture":
                x_data.append(entry.created)
                y_data.append(int(entry.value))
                xbar_data.append(entry.created)
                ybar_data.append(0)
            elif entry.kind == "pump_time":
                xbar_data.append(entry.created)
                ybar_data.append(int(entry.value))
        layout = {'title': 'Moisture Events',
                  'hovermode': 'closest'}
        layout['xaxis'] = {'title': 'Time', 'type': 'date', 'autorange': True}
        layout['yaxis1'] = {'title': 'Moisture %',
                            'type': 'linear',
                            'autorange': True,
                            'side': 'left',
                            'titlefont': {'color': 'orange'},
                            'tickfont': {'color': 'orange'}}
        layout['yaxis2'] = {'title': 'Pumping duration',
                            'type': 'linear',
                            'autorange': True,
                            'side': 'right',
                            'overlaying': 'y',
                            'anchor': 'x',
                            'titlefont': {'color': 'red'},
                            'tickfont': {'color': 'red'}}

        traces = [Scatter(x=x_data, y=y_data,
                         mode='lines', name='moisture level',
                         opacity=0.8, marker_color='green'),
                 Bar(x=xbar_data, y=ybar_data, name='pumping time',
                     yaxis='y2')]
        fig = go.Figure(data=traces, layout=layout)
        plot_div = plot(fig, output_type='div', include_plotlyjs=False)
        return Response({'plot_div': plot_div}, template_name='plot.html')

    def perform_create(self, serializer):
        serializer.save(owner=self.request.user)

    @action(methods=['post'], detail=False, permission_classes=[permissions.IsAuthenticated])
    def alert(self, request, *args, **kwargs):
        try:
            email = EmailMessage(
              'Moisture alert',
              ("This is an alert from your plant moisture management system. It "
              "is sent when the moisturing does not occur as expected. Please "
              "check your system."),
              settings.EMAIL_HOST_USER,
              [settings.EMAIL_HOST_USER],)
            email.send()
        except Exception as e:
            print(f"Catch exception {e} in alert mail sending")
        return Response()

class UserViewSet(viewsets.ReadOnlyModelViewSet):
    """
    This viewset automatically provides `list` and `retrieve` actions.
    """
    queryset = User.objects.all()
    serializer_class = UserSerializer
